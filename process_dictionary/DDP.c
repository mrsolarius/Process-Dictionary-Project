#include "DDP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DDP_Errno = -1;
const char *DDP_errList[] = {
        "Not a DDP Frame",
        "CMD not match with frame",
        "Error flag placed but data was found",
        "Success flag placed but no data was found",
        "Not found flag placed but this cmd can't be not found",
        "The length don't match with data",
        "No end flag in stream",
        "Not a ASK FRAME",
        "Not a ACQUITTEMENT FRAME",
        "The value can't be null"
};

char evaluateType(const unsigned char frame) {
    switch (frame) {
        case C_SET:
        case A_SET:
        case C_LOOKUP:
        case A_LOOKUP:
        case C_DUMP:
        case A_DUMP:
        case C_EXIT:
            break;
        default:
            DDP_Errno = ENOTDDP;
            return -1;
    }
    //On applique le mask sur le characther pour récupérer le type indiquer en deuxème positon
    // Exemeple
    // 0xA1 = 0b1010 0001
    // 0x0F = 0b0000 1111 on applique le masque
    //      & 0b0000 0001 avec le & bit à bit on obtient uniquement l'octet de point faible
    // soit la valeur du type de trame
    return frame & 0x0F;
}

PAskFrame decodeAskFrame(unsigned char *frame) {
    //Definition de la structure renvoyer en cas d'erreur
    PAskFrame error = (PAskFrame) malloc(sizeof(AskFrame));
    error->cmd = 0xff;

    char typeFrame = evaluateType(frame[0]);
    if (typeFrame == ACQUITTAL) {
        DDP_Errno = ENOTASK;
        return error;
    } else if (typeFrame == -1) {
        return error;
    }
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    unsigned int frameLength = strlen((char *) frame);
    //On teste la taille de la frame
    if (frameLength == 2) {
        //Si la taille et de 2 alors 1 commande autorisé
        if (frame[0] == C_SET || frame[0] == C_LOOKUP) {
            DDP_Errno = EBADCMD;
            return error;
        }
        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[1] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            return error;
        }

        askFrame->cmd = frame[0];
        askFrame->val = 0xff;
    } else if (frameLength == 4) {
        //Si la taille et de 4 alors 3 commande autorisé
        if (frame[0] == C_EXIT || frame[0] == C_DUMP ) {
            DDP_Errno = EBADCMD;
            return error;
        }
        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[3] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            return error;
        }

        askFrame->cmd = (unsigned char) frame[0];
        //On décale de un octer la premier case mémoire -1 puis on y concataine les bite de la deuxème trame -1
        //Pour éviter que strlen s'arrete de compter les ellement car il est tomber sur un octer à 0
        //on ajoute 1 au deux octer dans encodeAskframe que l'on doit retrancher ici
        askFrame->val = ((frame[1] - 1) << 8) + (unsigned char) frame[2] - 1;
    } else {
        //Si la taille ne corespond pas du tous ce n'est pas une trame DDP
        DDP_Errno = ENOTDDP;
        return error;
    }
    free(error);
    return askFrame;
}

PAcquittalFrame decodeAcquittalFrame(const unsigned char *frame, unsigned int frameLength) {
    //Definition de la structure renvoyer en cas d'erreur
    PAcquittalFrame error = (PAcquittalFrame) malloc(sizeof(AcquittalFrame));
    error->cmd = 0xff;

    char typeFrame = evaluateType(frame[0]);
    if (typeFrame == ASK) {
        DDP_Errno = ENOTAQIT;
        return error;
    } else if (typeFrame == -1) {
        return error;
    }
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(AcquittalFrame));
    //On teste la taille de la frame
    if (frameLength == 4) {
        switch (frame[0]) {
            case A_SET:
                //Un set ne peut pas avoir l'erreur notfound
                if (frame[2] == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    return error;
                }
                break;
            case A_DUMP:
                //Un dump ne peut pas avoir l'erreur notfound
                if (frame[2] == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    return error;
                }
                break;
            case A_LOOKUP:
                //Un lookup ne peut pas être un success sans données
                if (frame[2] == SUCCESS) {
                    DDP_Errno = EWRONGSFLAG;
                    return error;
                }
                break;
            default:
                DDP_Errno = EBADCMD;
                return error;
        }

        //Verification des flag d'erreurs
        switch (frame[2]) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                return error;
        }

        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[3] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            return error;
        }


        acquittalFrame->cmd = (unsigned char) frame[0];
        //pour eviter d'avoir une valeur null on à ajouter 1 dans l'encodage de la trame que l'on retranche ici
        acquittalFrame->nodeID = (unsigned char) frame[1] - 1;
        acquittalFrame->errorFlag = (unsigned char) frame[2];
        acquittalFrame->dataLength = 0;
        acquittalFrame->data = NULL;
    } else if (frameLength > 5) {
        if (frame[0] == A_SET || frame[0] == A_DUMP) {
            DDP_Errno = EBADCMD;
            return error;
        }

        //Verification des flag du protocole
        switch (frame[2]) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                return error;
        }

        //Une trame de lookup doit être un success si il y as de là data
        if (frame[2] != SUCCESS) {
            DDP_Errno = EWRONGEFLAG;
            return error;
        }

        //Verification de la fin de la frame
        if (frame[frameLength - 1] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            return error;
        }

        //Récupération de la longeur de la data
        //On décale de un octer la premier case mémoire -1 puis on y concataine les bite de la deuxème trame -1
        //Pour éviter que strlen s'arrete de compter les ellement car il est tomber sur un octer à 0
        //on ajoute 1 au deux octer dans encodeAquitallframe que l'on doit retrancher ici
        unsigned int length = ((frame[3] - 1) << 8) + frame[4] - 1;
        //On verifie quela taille de la trame et bien egale à la taile indiquer plus 5
        if (frameLength - length != 6) {
            DDP_Errno = EWRONGLEN;
            return error;
        }

        acquittalFrame->data = malloc(sizeof(char) * length);
        for (int i = 5, j = 0; i < frameLength - 1; i++, j++) {
            acquittalFrame->data[j] = frame[i];
        }

        acquittalFrame->cmd = frame[0];
        //pour eviter d'avoir une valeur null on à ajouter 1 dans l'encodage de la trame que l'on retranche ici
        acquittalFrame->nodeID = frame[1] - 1;
        acquittalFrame->errorFlag = frame[2];
        acquittalFrame->dataLength = length;
    } else {
        //Si la taille ne corespond pas du tous ce n'est pas une trame DDP
        DDP_Errno = ENOTDDP;
        return error;
    }
    free(error);
    return acquittalFrame;
}

unsigned char *encodeAskFrame(PAskFrame askFrame) {
    char length = 0;
    unsigned char *frame;
    //Definition du renvoi d'erreur
    unsigned char *error = malloc(sizeof(unsigned char));
    error[0] = 0xff;
    if (evaluateType(askFrame->cmd) == -1) {
        return error;
    } else if (evaluateType(askFrame->cmd) != ASK) {
        DDP_Errno = EBADCMD;
        return error;
    }
    if(askFrame->cmd==C_EXIT || askFrame->cmd==C_DUMP) {
        length = 2;
    }else{
        length = 4;
    }

    frame = malloc(sizeof(unsigned char) * length);
    if (length == 2) {
        frame[0] = askFrame->cmd;
        frame[1] = END_FRAME;
    } else if (length == 4) {
        frame[0] = askFrame->cmd;
        //Si la valeur est null (null ici veut dire 255)
        if (askFrame->val == 0xff) {
            DDP_Errno = ENOTNULL;
            return error;
        }
        frame[1] = ((askFrame->val >> 8) & 0xFF)+1;
        frame[2] = (askFrame->val + 1 ) & 0xFF;
        frame[3] = END_FRAME;
    } else {
        DDP_Errno = ENOTDDP;
        return error;
    }
    free(error);
    return frame;
}

unsigned char * encodeAcquittalFrame(PAcquittalFrame acquittalFrame) {
    unsigned int length;
    unsigned char *frame;
    //Definition du renvoi d'erreur
    unsigned char *error = malloc(sizeof(unsigned char));
    error[0] = 0xff;

    if (evaluateType(acquittalFrame->cmd) == -1) {
        return error;
    } else if (evaluateType(acquittalFrame->cmd) != ACQUITTAL) {
        DDP_Errno = EBADCMD;
        return error;
    }

    if(acquittalFrame->dataLength>0){
        length =  7 + acquittalFrame->dataLength;
        frame = malloc(sizeof(char) * length);
        if (acquittalFrame->cmd == A_SET || acquittalFrame->cmd == A_DUMP) {
            DDP_Errno = EBADCMD;
            return error;
        }

        //Verification des flag du protocole
        switch (acquittalFrame->errorFlag) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                return error;
        }

        //Une trame de lookup doit être un success si il y as de là data
        if (acquittalFrame->errorFlag != SUCCESS) {
            DDP_Errno = EWRONGEFLAG;
            return error;
        }

        //Parcoure des data pour ocnnaitre la longeur réel
        unsigned int checkLen = 0;
        while (acquittalFrame->data[checkLen]!=0){
            checkLen++;
        }

        //Si la longeur ne correspond pas à celle indiquer on envoie une erreur
        if(checkLen!=acquittalFrame->dataLength) {
            DDP_Errno = EWRONGLEN;
            return error;
        }

        frame[0]=acquittalFrame->cmd;
        //pour eviter d'avoir une valeur null on à ajouter 1 dans l'encodage de la trame que l'on retranche ici
        frame[1]= acquittalFrame->nodeID + 1;
        frame[2] = acquittalFrame->errorFlag;
        //Séparation de l'unsigned int en 2 char couper en octer
        frame[3] = ((acquittalFrame->dataLength >> 8) & 0xFF)+1;
        frame[4] = (acquittalFrame->dataLength + 1 ) & 0xFF;
        //Parcoure des données de la structure est inisialisation de celle ci dans la trame
        for (unsigned int  i = 5, j = 0; j <= acquittalFrame->dataLength; i++, j++) {
            if(j < acquittalFrame->dataLength) {
                frame[i] = acquittalFrame->data[j];
            }else {
                frame[i] = END_FRAME;
                frame[i + 1] = '\0';
            }
        }
        //On termine par le end frame (défini par i+1 pour être à la fin du tableau)
        //frame[acquittalFrame->dataLength]=END_FRAME;
    } else{
        length = 4;
        frame = malloc(sizeof(char) * length);

        //Verification des flag du protocole
        switch (acquittalFrame->errorFlag) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                return error;
        }

        //Verification de la commande utiliser
        switch (acquittalFrame->cmd) {
            case A_SET:
                //Un set ne peut pas avoir l'erreur notfound
                if (acquittalFrame->errorFlag == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    return error;
                }
                break;
            case A_DUMP:
                //Un dump ne peut pas avoir l'erreur notfound
                if (acquittalFrame->errorFlag == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    return error;
                }
                break;
            case A_LOOKUP:
                //Un lookup ne peut pas être un success sans données
                if (acquittalFrame->errorFlag == SUCCESS) {
                    DDP_Errno = EWRONGSFLAG;
                    return error;
                }
                break;
            default:
                DDP_Errno = EBADCMD;
                return error;
        }

        frame[0]=acquittalFrame->cmd;
        //pour eviter d'avoir une valeur null on à ajouter 1 dans l'encodage de la trame que l'on retranche ici
        frame[1]= acquittalFrame->nodeID + 1;
        frame[2] = acquittalFrame->errorFlag;
        frame[3] = END_FRAME;
    }
    free(error);
    return frame;
}

void DDP_perror(char *data) {
    if (DDP_Errno >= 0 && DDP_Errno <= 8) {
        fprintf(stderr, "\n%s : %s\n", data, DDP_errList[DDP_Errno]);
    } else {
        fprintf(stderr, "\n%s : Success\n", data);
    }
}

unsigned char getCharLength(unsigned char * val){
    unsigned char length = 0;
    while(*val) {
        length++;
        val++;
    }
    return length;
}
//
// Created by louis on 12/10/2021.
//

#include "DDP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DDP_Errno = -1;
const char *DDP_errList[] = {
        "Not a DDP Frame",
        "CMD not match with frame",
        "Node isn't valid",
        "Error flag placed but data was found",
        "Success flag placed but no data was found",
        "Not found flag placed but this cmd can't be not found",
        "The length don't match with data",
        "No end flag in stream",
        "Not a ASK FRAME",
        "Not a ACQUITTEMENT FRAME",
        "The value can't be null"
};;

char evaluateType(char *frame) {
    switch (frame[0]) {
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
            exit(-1);
    }
    //On applique le mask sur le characther pour récupérer le type indiquer en deuxème positon
    return frame[0] & (char)0x0F;
}

PAskFrame decodeAskFrame(char *frame) {
    char typeFrame = evaluateType(frame);
    if (typeFrame == ACQUITTAL) {
        DDP_Errno = ENOTASK;
        exit(-1);
    } else if (typeFrame == -1) {
        exit(-1);
    }
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    unsigned int frameLength = strlen(frame);
    //On teste la taille de la frame
    if (frameLength == 2) {
        //Si la taille et de 2 alors 2 commande autorisé
        switch (frame[0]) {
            case C_EXIT:
                break;
            default:
                DDP_Errno = EBADCMD;
                exit(-1);
        }
        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[1] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            exit(-1);
        }

        askFrame->cmd = frame[0];
        askFrame->val = 0xff;
    } else if (frameLength == 4) {
        //Si la taille et de 4 alors 2 commande autorisé
        switch (frame[0]) {
            case C_SET:
            case C_LOOKUP:
            case C_DUMP:
                break;
            default:
                DDP_Errno = EBADCMD;
                exit(-1);
        }
        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[3] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            exit(-1);
        }

        askFrame->cmd = (unsigned char) frame[0];
        askFrame->val = (frame[1] << 8) + frame[2];
    } else {
        //Si la taille ne corespond pas du tous ce n'est pas une trame DDP
        DDP_Errno = ENOTDDP;
        exit(-1);
    }
    return askFrame;
}

PAcquittalFrame decodeAcquittalFrame(char *frame) {
    char typeFrame = evaluateType(frame);
    if (typeFrame == ASK) {
        DDP_Errno = ENOTAQIT;
        exit(-1);
    } else if (typeFrame == -1) {
        exit(-1);
    }
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(AcquittalFrame));
    unsigned int frameLength = strlen(frame);
    //On teste la taille de la frame
    if (frameLength == 4) {
        switch (frame[0]) {
            case A_SET:
                //Un set ne peut pas avoir l'erreur notfound
                if (frame[2] == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    exit(-1);
                }
                break;
            case A_DUMP:
                //Un dump ne peut pas avoir l'erreur notfound
                if (frame[2] == NOT_FOUND) {
                    DDP_Errno = EWRONGNFLAG;
                    exit(-1);
                }
                break;
            case A_LOOKUP:
                //Un lookup ne peut pas être un success sans données
                if (frame[2] == SUCCESS) {
                    DDP_Errno = EWRONGSFLAG;
                    exit(-1);
                }
                break;
            default:
                DDP_Errno = EBADCMD;
                exit(-1);
        }

        //Verification de l'identifiant du node
        if (frame[1] > 0 && frame[1] < 255) {
            DDP_Errno = EBADNODE;
            exit(-1);
        }

        //Verification des flag d'erreurs
        switch (frame[2]) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                exit(-1);
        }

        //Si la fin de trame ne correspond pas au flag de fin alors error
        if (frame[3] != END_FRAME) {
            DDP_Errno = ENOENDFLAG;
            exit(-1);
        }


        acquittalFrame->cmd = (unsigned char) frame[0];
        acquittalFrame->nodeID = (unsigned char) frame[1];
        acquittalFrame->errorFlag = (unsigned char) frame[2];
        acquittalFrame->dataLength = 0;
        acquittalFrame->data = NULL;
    } else if (frameLength > 5) {
        if (frame[0] != A_LOOKUP) {
            DDP_Errno = ENOTDDP;
        }

        //Verification de l'identifiant du node
        if (frame[1] > 0 && frame[1] < 255) {
            DDP_Errno = EBADNODE;
            exit(-1);
        }

        //Verification des flag du protocole
        switch (frame[2]) {
            case SUCCESS:
            case NOT_FOUND:
            case INTERNAL_ERROR:
                break;
            default:
                DDP_Errno = ENOTDDP;
                exit(-1);
        }

        //Une trame de lookup doit être un success si il y as de là data
        if (frame[2] != SUCCESS) {
            DDP_Errno = EWRONGEFLAG;
            exit(-1);
        }

        //Verification de la fin de la frame
        if (frame[frameLength - 1]) {
            DDP_Errno = ENOENDFLAG;
            exit(-1);
        }

        //Récupération de la longeur de la data
        unsigned int length = (frame[3] << 8) + frame[4];

        //On verifie quela taille de la trame et bien egale à la taile indiquer plus 5
        if (length + 5 != frameLength) {
            DDP_Errno = EWRONGLEN;
            exit(-1);
        }

        char *data = malloc(sizeof(char) * length);

        for (int i = 0; i < length - 2; i++) {
            data[i] = (frame[i] << 8) + frame[i + 1];
        }

        acquittalFrame->cmd = frame[0];
        acquittalFrame->nodeID = frame[1];
        acquittalFrame->errorFlag = frame[2];
        acquittalFrame->dataLength = length;
        acquittalFrame->data = data;
    }

    return acquittalFrame;
}

char *encodeAskFrame(PAskFrame askFrame) {
    int length = 0;

    if (evaluateType((char *) &askFrame->cmd) != ASK) {
        DDP_Errno = ENOTASK;
        exit(-1);
    }

    switch ((char) askFrame->cmd) {
        case C_EXIT:
            length = 2;
            break;
        case C_SET:
        case C_LOOKUP:
        case C_DUMP:
            length = 4;
            break;
        default:
            DDP_Errno = EBADCMD;
            exit(-1);
    }

    char *frame = malloc(sizeof(char) * length);
    switch (length) {
        case 2:
            frame[0] = askFrame->cmd;
            frame[1] = END_FRAME;
            break;
        case 4:
            frame[0]=askFrame->cmd;
            //Si la valeur est null (null ici veut dire 255)
            if(askFrame->val==0xff){
                DDP_Errno = ENOTNULL;
                exit(-1);
            }
            frame[1] = askFrame->val & 0xFF;
            frame[2] = (askFrame->val >> 8) & 0xFF;
            frame[3] = END_FRAME;
    }

    return frame;
}

char *encodeAcquittalFrame(PAcquittalFrame acquittalFrame) {
    return (char *) (char) -1;
}

void DDP_perror(char *data) {
    if (DDP_Errno >= 0 && DDP_Errno <= 8) {
        fprintf(stderr, "%s : %s\n", data, DDP_errList[DDP_Errno]);
    } else {
        fprintf(stderr, "%s : Success\n", data);
    }
}
//
// Created by louis on 17/10/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include "controller.h"
#include "node.h"

unsigned int countAcquittal = 0;

void runController(int nbNodes) {
    //Allocation de la mémoire pour un tableau de nbNode * 2 car deux partie dans un pipe
    int **pipeArr = (int **) malloc(nbNodes * sizeof(int) * 2);
    //Allocation de la mémoire pour le pipe du controller
    int *pipeCtrl = (int *) malloc(2 * sizeof(int));
    initPipes(nbNodes, pipeCtrl, pipeArr);
    launchPipes(nbNodes, pipeCtrl, pipeArr);
    int *pipeCtrlWrite = pipeArr[nbNodes - 1];
    close(pipeCtrlWrite[0]);
    close(pipeCtrl[1]);
    cmdLauncher(nbNodes, pipeCtrlWrite, pipeCtrl);
    close(pipeCtrlWrite[1]);
    close(pipeCtrl[0]);
    freeNodes(nbNodes, pipeCtrl, pipeArr);
}

void initPipes(int nbNodes, int *pipeCtrl, int **pipeArr) {
    for (int i = 0; i < nbNodes; i++) {
        //Allocation de la mémoire pour la ligne de la pipe courent
        pipeArr[i] = (int *) malloc(2 * sizeof(int));
        if (pipe(pipeArr[i]) == -1) {
            perror("controller.c::initPipes() call pipe()");
            exit(-1);
        }
    }

    //On ouvre le pipe en direction du controller
    if (pipe(pipeCtrl) == -1) {
        perror("controller.c::initPipes() call pipe()");
        exit(-1);
    }
}

void launchPipes(int nbNodes, int *pipeCtrl, int **pipeArr){
    for (int i = 0; i < nbNodes; i++) // Générer n node
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        int val = fork();
        if (val == 0) {
            if (i == 0) {
                runNode(i, nbNodes, pipeCtrl, pipeArr[nbNodes - 1], pipeArr[i]);
            } else {
                runNode(i, nbNodes, pipeCtrl, pipeArr[i - 1], pipeArr[i]);
            }
        }else if (val == -1){
            perror("controller.c::launchPipes() call fork()");
            exit(-1);
        }
    }
}

void freeNodes(int nbNodes, int *pipeCtrl, int **pipeArr) {
    //On libère du heap les tableaux et on attend les fis
    for (int i = 0; i < nbNodes; i++) {
        close(pipeArr[i][0]);
        close(pipeArr[i][1]);
        free(pipeArr[i]);
        wait(NULL);
    }
    free(pipeArr);
    free(pipeCtrl);
}

void cmdLauncher(int nbNodes, int *pipeCtrlWrite, int *pipeCtrlRead) {
    int exit = 0, val = 99;
    //Boucle principale du programme tant que EXIT egale 0
    while (exit == 0) {
        printf("Saisir commande (0 = exit, 1 = set, 2 = lookup, 3 = dump):\n");
        scanf("%d", &val);
        switch (val) {
            case 0:
                launchExit(pipeCtrlWrite);
                exit = 1;
                break;
            case 1:
                askKey(C_SET,pipeCtrlWrite);
                readAcquittal(pipeCtrlRead,nbNodes);
                break;
            case 2:
                askKey(C_LOOKUP,pipeCtrlWrite);
                readAcquittal(pipeCtrlRead,nbNodes);
                break;
            case 3:
                launchAsk(pipeCtrlWrite, C_DUMP, nbNodes);
                //On attend que tous les node soit bien reçu grace a la variable global count acquittal
                //Si ce n'est pas le cas on continue à lire dans le pipe de lecture du controller
                while(countAcquittal<nbNodes){
                    readAcquittal(pipeCtrlRead,nbNodes);
                }
                printf("Toutes les commandes ont effectué leurs dump\n");
                break;
            default:
                printf("Votre commande n'est pas reconnue veuillez recommencer\n");
                break;
        }
        //Aprés chaque fin de traitement on reset le count des acquittal pour ne pas perturber les suivant
        countAcquittal = 0;
    }
    printf("bye bye !\n");
}

void askKey(unsigned char cmd,int * pipeCtrlWrite){
    unsigned int key;
    //on demande à l'utilisateur de saisir une clef
    printf("Saisir la cle (decimal number): ");
    scanf("%d", &key);
    //clef que l'on envoie avec la commande demander à la fonction launch ask
    launchAsk(pipeCtrlWrite, cmd, key);
}

void readAcquittal(int *pipeCtrlRead,unsigned int totalNodes) {
    int bytes = 4096;
    unsigned char frame[bytes];
    //On utilise memset pour nettoyer les valeurs indésirables de notre tableau qui pourrais trainer dans la mémoire
    //On le defines donc entièrement à 0
    memset(frame, 0, bytes);
    if(read(pipeCtrlRead[0],frame,bytes)==-1){
        perror("controller.c::readAcquittal() call read()");
        exit(-1);
    }
    //On traite les trames concaténer ici
    PAcquittalFrame acquittalFinal = deserialization(frame, bytes,totalNodes);
    if(acquittalFinal->cmd==0xff){
        DDP_perror("controller.c::readAcquittal() call decodeAcquittalFrame()");
        exit(-1);
    }
    //Puis on envois la structure trouver à la fonction de traitement des acquittements
    acquittalAction(acquittalFinal);
}

int launchExit(int *pipeCtrlWrite) {
    PAskFrame exitFrame = (PAskFrame) malloc(sizeof(AskFrame));
    exitFrame->cmd = C_EXIT;
    //Generation d'une trame de exit
    unsigned char *frame = encodeAskFrame(exitFrame);
    if (frame[0] == 0xff) {
        DDP_perror("controller.c::launchExit() call encodeAskFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    // envoie de la payload
    if (write(pipeCtrlWrite[1], frame, frameLength) == -1) {
        perror("controller.c::launchExit() call write()");
        exit(-1);
    }
    return 0;
}

int launchAsk(int *pipeCtrlWrite, unsigned char cmd,unsigned int val) {
    PAskFrame dumpFrame = (PAskFrame) malloc(sizeof(AskFrame));
    //Generation d'une trame de ask avec les parametres fournie
    dumpFrame->cmd = cmd;
    dumpFrame->val = val;
    //génération du tableau (payload)
    unsigned char *frame = encodeAskFrame(dumpFrame);
    if (frame[0] == 0xff) {
        DDP_perror("controller.c::launchAsk() call encodeAskFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    //envoie de la payload
    if (write(pipeCtrlWrite[1], frame, frameLength) == -1) {
        perror("controller.c::launchAsk() call write()");
        exit(-1);
    }
    return 0;
}

PAcquittalFrame deserialization(unsigned char * frame, unsigned int bufferSize,unsigned int totalNode){
    PAcquittalFrame acquittalFinal = (PAcquittalFrame) malloc(sizeof (AcquittalFrame));
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof (AcquittalFrame));
    //On parcour le buffer
    for (int i = 0, len = 1, start = 0; i < bufferSize; ++i, ++len) {
        unsigned char *str;
        //On alloue de la memoire au string de travail
        str = malloc(sizeof(unsigned char)*len+1);
        /**
         * Comme les frame peuvent être lu en même temps dans le même buffer,
         * ici on se décale petite à petite pour découper la trame grace à la fonction strncpy()
         * qui copy un string dans un autre avec une longeur definis
         * ici on utilise le l'addition de pointeur pour générer un décalage et donc compier
         * une parti tronqué du string
         *
         * Info sur les variable et le déroulement :
         * start correspond au dernier point d'encrage et len à la longeur incrémenter à chaque étape
         * par exemple pour la chaine suivante : {0xd2,0xe8,0x20,0x04,0xd2,0xe9,0x20,0x04}
         * nous allons nous déplacer de la façon suivanter :
         * 1 : str:{0xd2} start:0 len:1
         * 2 : str:{0xd2,0xe8} start:0 len:2
         * [...]
         * 4 : str:{0xd2,0xe8,0x20,0x04} start:0 len:4
         * Ici la fonction decode acquittal ne renverra pas d'erreur car str est une trame correcte
         * start sera egale à start + len donc on aura :
         * 5 : str:{0xd2} start:4 len:1
         * 6 : str:{0xd2,0xe9} start:4 len:2
         * Se qui permet de compter la trame suivante
         */
        strncpy((char *)str, (char *)frame + start, len);
        acquittalFrame = decodeAcquittalFrame(str,len);
        if (acquittalFrame->cmd != 0xff) {
            // ici si j'avais eu le temps j'aurais préféré stoker toute les structure d'acquittement dans une
            // liste chainer ou un tableau pour éventuellement afin de les renvoyer et de les traiter ailleurs plus tard
            //
            // en attendant je copie la structure trouver dans la structure final renvoyer par la fonction
            // la copie est obligatoire car acquittalFrame et free() à chaque itération
            memcpy(acquittalFinal, acquittalFrame, sizeof(AcquittalFrame));
            start += len;
            len = 1;
            //On incrémente la variable global qui permet de savoir combien de trame on était lu depuis le dernier rest
            countAcquittal++;
        }
        //On liber toutes nos valeur
        free(str);
        //surtous le aquitalframe pour eviter d'avoir des information résiduel à la prochaine itération
        free(acquittalFrame);
    }
    //On retire le dernier flag positioner dans DDP erno car beaucoup d'erreur on put être cause lors de ce traitement
    DDP_Errno = -1;
    return acquittalFinal;
}

void acquittalAction(PAcquittalFrame incomingAcquittal){
    //On réagis en fonction des commande renvoyer par la trame d'aquitement
    //Juste à suivre la logique du switch case et des if
    switch (incomingAcquittal->cmd) {
        case A_SET:
            if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }else {
                printf("Votre donnée à bien était enregistrer (noeud n°%d)\n", incomingAcquittal->nodeID);
            }
            break;
        case A_LOOKUP:
            if(incomingAcquittal->errorFlag == NOT_FOUND){
                printf("Pas de valeur trouver\n");
            }else if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }else{
                printf("Valeur (noeud n°%d) = %s\n",incomingAcquittal->nodeID,incomingAcquittal->data);
            }
            break;
        case A_DUMP:
            if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }
            break;
    }
}
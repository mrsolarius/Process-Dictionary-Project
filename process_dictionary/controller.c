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

void launchPipes(int nbNodes, int *pipeCtrlRead, int **pipeArr){
    for (int i = 0; i < nbNodes; i++) // Générer n node
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        if (fork() == 0) {
            //printf("\nfork:%d",i);
            if (i == 0) {
                runNode(i, nbNodes, pipeCtrlRead, pipeArr[nbNodes - 1], pipeArr[i]);
            } else {
                runNode(i, nbNodes, pipeCtrlRead, pipeArr[i - 1], pipeArr[i]);
            }
        }
    }
}

void freeNodes(int nbNodes, int *pipeCtrl, int **pipeArr) {
    //On libère du heap les tableaux et on attend les fis
    for (int i = 0; i < nbNodes; i++) {
        free(pipeArr[i]);
        wait(NULL);
    }
    free(pipeArr);
    free(pipeCtrl);
}

void cmdLauncher(int nbNodes, int *pipeCtrlWrite, int *pipeCtrlRead) {
    int exit = 0, val = 99;
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
                readAcquittal(pipeCtrlRead);
                break;
            case 2:
                askKey(C_LOOKUP,pipeCtrlWrite);
                readAcquittal(pipeCtrlRead);
                break;
            case 3:
                launchAsk(pipeCtrlWrite, C_DUMP, nbNodes);
                while(countAcquittal<nbNodes){
                    readAcquittal(pipeCtrlRead);
                }
                printf("Toutes les commandes on effectuer leur dump\n");
                break;
            default:
                printf("Votre commande n'est pas reconue veuillez recomancer\n");
                break;
        }
        countAcquittal = 0;
    }
    printf("bye bye !\n");
}

void askKey(unsigned char cmd,int * pipeCtrlWrite){
    unsigned int key;
    printf("Saisir la cle (decimal number): ");
    scanf("%d", &key);
    launchAsk(pipeCtrlWrite, cmd, key);
}

void readAcquittal(int *pipeCtrlRead) {
    int bytes = 128;
    unsigned char frame[bytes];
    if(read(pipeCtrlRead[0],frame,bytes)==-1){
        perror("controller.c::readAcquittal() call read()");
        exit(-1);
    }
    PAcquittalFrame acquittalFinal = deserialization(frame, bytes);
    if(acquittalFinal->cmd==0xff){
        DDP_perror("controller.c::readAcquittal() call decodeAcquittalFrame()");
        exit(-1);
    }
    acquittalAction(acquittalFinal);
}

int launchExit(int *pipeCtrlWrite) {
    PAskFrame exitFrame = (PAskFrame) malloc(sizeof(AskFrame));
    exitFrame->cmd = C_EXIT;
    unsigned char *frame = encodeAskFrame(exitFrame);
    if (frame[0] == 0xff) {
        DDP_perror("controller.c::launchExit() call encodeAskFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    if (write(pipeCtrlWrite[1], frame, frameLength) == -1) {
        perror("controller.c::launchExit() call write()");
        exit(-1);
    }
    return 0;
}

int launchAsk(int *pipeCtrlWrite, unsigned char cmd,unsigned int val) {
    PAskFrame dumpFrame = (PAskFrame) malloc(sizeof(AskFrame));
    dumpFrame->cmd = cmd;
    dumpFrame->val = val;
    unsigned char *frame = encodeAskFrame(dumpFrame);
    if (frame[0] == 0xff) {
        DDP_perror("controller.c::launchAsk() call encodeAskFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    if (write(pipeCtrlWrite[1], frame, frameLength) == -1) {
        perror("controller.c::launchAsk() call write()");
        exit(-1);
    }
    return 0;
}

PAcquittalFrame deserialization(unsigned char * frame, unsigned int bufferSize){

    PAcquittalFrame acquittalFinal = (PAcquittalFrame) malloc(sizeof (AcquittalFrame));
    unsigned int acquittalLen = 0;
    unsigned int passed = 0;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof (AcquittalFrame));
    for (int i = 0, len = 0, start = 0; i < bufferSize; ++i, ++len) {
        unsigned char *str;
        str = malloc(sizeof(unsigned char));
        strncpy(str, frame + start, len);
        acquittalFrame = NULL;
        acquittalFrame = decodeAcquittalFrame(str);
        if (acquittalFrame->cmd != 0xff) {
            if (acquittalLen == 0) {
                acquittalFinal = acquittalFrame;
            }
            //printf("Reception de l'aquitement du neud n°%d\n",acquittalFrame->nodeID);
            start = len;
            len = 0;
            acquittalLen++;
            countAcquittal++;
            DDP_Errno = -1;
            passed=1;
        }
        free(str);
    }
    free(acquittalFrame);
    return acquittalFinal;
}

void acquittalAction(PAcquittalFrame incomingAcquittal){
    switch (incomingAcquittal->cmd) {
        case A_SET:
            if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }else {
                printf("Votre donnée à bien était enregistreer par le noeud numéro %d\n", incomingAcquittal->nodeID);
            }
            break;
        case A_LOOKUP:
            if(incomingAcquittal->errorFlag == NOT_FOUND){
                printf("Pas de valeur trouver\n");
            }else if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }else{
                printf("Valeur (neud n°%d) = %s\n",incomingAcquittal->nodeID,incomingAcquittal->data);
            }
            break;
        case A_DUMP:
            if(incomingAcquittal->errorFlag == INTERNAL_ERROR){
                printf("Le processus %d à subit une erreur lors de sont execution\n",incomingAcquittal->nodeID);
            }
            break;
    }
}
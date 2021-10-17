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

int runController(int nbNodes) {
    printf("ctrl:\n");
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
    printf("ici");
    /*
    int *pipeCtrlWrite = pipeArr[nbNodes - 1];
    close(pipeCtrlWrite[0]);
    close(pipeCtrl[1]);
    //cmdLauncher(nbNodes, pipeCtrlWrite, pipeCtrl);
    //Injection de la premier valeur au node 0

    /*
    int val = 500;
    int buffCtrl;
    //On lis les message envoyer par les fis
    while (read(pipeCtrl[0], &buffCtrl, sizeof(int)) > 0 && buffCtrl != nbNodes - 1) {
        printf("Signal receive : %d\n", buffCtrl);
    }
    printf("Signal receive : %d\n", buffCtrl);*/
    freeNodes(nbNodes, pipeCtrl, pipeArr);
    return 0;
}

void initPipes(int nbNodes, int *pipeCtrl, int **pipeArr) {
    printf("initPipes\n");
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
    printf("launchPipes\n");

    for (int i = 0; i < nbNodes; i++) // Générer n node
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        if (fork() == 0) {
            //printf("\nfork:%d",i);
            if (i == 0) {
                runNode(i, pipeCtrlRead, pipeArr[nbNodes - 1], pipeArr[i]);
            } else {
                runNode(i, pipeCtrlRead, pipeArr[i - 1], pipeArr[i]);
            }
        }
    }
}

int freeNodes(int nbNodes, int *pipeCtrl, int **pipeArr) {
    //On libère du heap les tableaux et on attend les fis
    for (int i = 0; i < nbNodes; i++) {
        free(pipeArr[i]);
        wait(NULL);
    }
    free(pipeArr);
    free(pipeCtrl);
}

int cmdLauncher(int nbNodes, int *pipeCtrlWrite, int *pipeCtrlRead) {
    int exit = 0, val = 99;
    while (exit == 0) {
        printf("Saisir commande (0 = exit, 1 = set, 2 = lookup, 3 = dump):\n");
        scanf("%d", &val);
        switch (val) {
            case 0:
                launchExit(pipeCtrlWrite);
                break;
            case 1:
                printf("set\n");
                exit = 1;
                break;
            case 2:
                printf("lookup\n");
                break;
            case 3:
                launchDump(pipeCtrlWrite,nbNodes);
                for(int i=0;i<nbNodes;i++) {
                    readAcquittal(pipeCtrlRead, nbNodes);
                }
                break;
            default:
                printf("Votre commande n'est pas reconue veuillez recomancer\n");
                break;
        }
    }
    printf("bye bye !\n");
}

int readAcquittal(int *pipeCtrlRead, int nbNodes){
    int bytes = 4096;
    char * frame = malloc(sizeof (char )*bytes);
    long res;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof (AcquittalFrame));
    res = read(pipeCtrlRead[0],frame,bytes);
    acquittalFrame = decodeAcquittalFrame(frame);
    printf("nodeID : %d, cmd:%x",acquittalFrame->nodeID, acquittalFrame->cmd);/*
    for(int i=0;i<nbNodes;i++){
    }*/
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

int launchDump(int *pipeCtrlWrite,int nbNodes) {
    PAskFrame dumpFrame = (PAskFrame) malloc(sizeof(AskFrame));
    dumpFrame->cmd = C_DUMP;
    dumpFrame->val = nbNodes;
    unsigned char *frame = encodeAskFrame(dumpFrame);
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
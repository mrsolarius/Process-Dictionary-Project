//
// Created by louis on 17/10/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

int runNode(int appID, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]){
    closePipes(pipeCtr,pipeRead,pipeWrite);

    int buffNode=0;
    long int res;

    //On Lis le tube du précédent du node
    res = read(pipeRead[0], &buffNode, sizeof(int));
    if (res == -1) perror("Probléme de lecture");
    //close(pipeRead[0]);

    //On Affiche le contenu du node précédent
    printf("id:%d buffNode:%d\n", appID, buffNode);
    //On l'incrémente de 1 pour s'assurer que l'opération et executer sur tous les node
    buffNode += 1;

    //on envois se nouveau contenue au node suivant
    res = write(pipeWrite[1], &buffNode, sizeof(int));
    if (res == -1)perror("Probléme d'écriture node");
    //close(pipeWrite[1]);


    //On écrit l'id du processus dans le tub en direction du controller
    res = write(pipeCtr[1], &appID, sizeof(int));
    if (res == -1)perror("Probléme d'écriture ctrl");
    //close(pipeCtr[1]);

    //on quite le process
    exit(0);
}

int closePipes(int * pipeCtr, int * pipeRead, int * pipeWrite){
    //Avant de commencer notre traitement on ferme les lecture/ecriture qui ne nous serve pas sur nos différent pipes
    //On ferme en lecture le pipe du controller
    close(pipeCtr[0]);
    //On ferme en ecriture le pipe de lecture
    close(pipeRead[1]);
    //On ferme en lecture le pipe d'écriture
    close(pipeWrite[0]);
}

int sendToController(PAcquittalFrame acquittalFrame, int pipeCtr[2]){
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    if(frame[0]==0xff){
        DDP_perror("node.c::sendToController() call encodeAcquittalFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    if(write(pipeCtr[1],frame,frameLength)==-1){
        perror("node.c::sendToController() call write()");
        exit(-1);
    }
    return 0;
}

int sendNextNode(PAskFrame askFrame,int pipeWrite[2]) {
    unsigned char * frame = encodeAskFrame(askFrame);
    if(frame[0]==0xff){
        DDP_perror("node.c::sendNextNode() call encodeAskFrame()");
        exit(-1);
    }
    unsigned int frameLength = strlen((char *) frame);
    if(write(pipeWrite[1],frame,frameLength)==-1){
        perror("node.c::sendNextNode() call write()");
        exit(-1);
    }
    return 0;
}
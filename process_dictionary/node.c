//
// Created by louis on 17/10/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include "node.h"
#include "table.h"

unsigned char* concat(unsigned char *s1, unsigned char *s2)
{
    printf("ici");
    unsigned char *result = malloc(strlen((char *)s1)+strlen((char *) s2)+1);//+1 pour la terminaison à 0
    strcpy((char *) result, (char *) s1);
    strcat((char *) result, (char *) s2);
    return result;
}

void runNode(int nodeID, int totalNode, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]){
    __pid_t pid = getpid();
    printf("runNode %d", nodeID);
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    Table_entry *dataBases = NULL;
    long res;
    closePipes(pipeCtr,pipeRead,pipeWrite);
    unsigned char *frame;
    do{
        frame = readFrame(pipeRead);
        askFrame = decodeAskFrame(frame);
        if(askFrame->cmd==0xff){
            DDP_perror("node.c::runNode() call decodeAskFrame()");
            exit(0);
        }
        switch (askFrame->cmd) {
            case C_SET:
                if(askFrame->val%totalNode-1){
                    char value[4000];
                    printf("Saisir la valeur (chaine de caracteres): ");
                    scanf("%s", &value);
                    store(&dataBases, askFrame->val, value);
                    acquittalFrame->cmd = A_SET;
                    acquittalFrame->nodeID = nodeID;
                    acquittalFrame->errorFlag=SUCCESS;
                    acquittalFrame->dataLength=0;
                    frame = encodeAcquittalFrame(acquittalFrame);
                    if(frame[0]==0xff){
                        DDP_perror("node.c::runNode() call encodeAcquittalFrame() (C_SET)");
                        exit(-1);
                    }
                    unsigned long len = strlen(( char *)frame);
                    res = write(pipeCtr[1],frame,len);
                    if (res == -1) {
                        perror("Probléme d'écriture node");
                        exit(-1);
                    }
                }else{
                    res = write(pipeWrite[1], frame, 4);
                    if (res == -1) {
                        perror("Probléme d'écriture node");
                    }
                }
                break;
            case C_LOOKUP:
                break;
            case C_DUMP:
                printf("dump %d", nodeID);
                if(totalNode-1 != nodeID){
                    res = write(pipeWrite[1], frame, 4);
                    if (res == -1) {
                        perror("Probléme d'écriture node");
                    }
                }
                display(dataBases);
                acquittalFrame->cmd = A_DUMP;
                acquittalFrame->nodeID = nodeID;
                acquittalFrame->errorFlag=SUCCESS;
                acquittalFrame->dataLength=0;
                frame = encodeAcquittalFrame(acquittalFrame);
                if(frame[0]==0xff){
                    DDP_perror("node.c::runNode() call encodeAcquittalFrame() (C_DUMP)");
                    exit(-1);
                }
                unsigned long len = strlen(( char *)frame);
                res = write(pipeCtr[1],frame,len);
                if (res == -1) {
                    perror("Probléme d'écriture node");
                    exit(-1);
                }
                break;
            case C_EXIT:
                res = write(pipeWrite[1], frame, 4);
                if (res == -1) {
                    perror("Probléme d'écriture node");
                    exit(-1);
                }
                break;
        }
        //printf("\nnode: %d", nodeID);
        //printf("\ncmd:%x val:%d ", askFrame->cmd, askFrame->val);

        printf("\n");
    }while (askFrame->cmd!=C_EXIT);
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

unsigned char * readFrame(int pipeRead[2]){
    printf("readFrame()");
    char nbBytes = 4;
    unsigned char * frame = malloc(sizeof (char)*nbBytes);
    long res=1;
    res = read(pipeRead[0], frame, nbBytes);
    //strcat(frame,buffTemp);
    /*
    while((res = read(pipeRead[0],&buffTemp,nbBytes))>0){
        frame = concat(frame,buffTemp);
        printf("là");
    }*/
    if (res == -1) {
        perror("node.c::runNode() call read()");
        exit(-1);
    }
    return frame;
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
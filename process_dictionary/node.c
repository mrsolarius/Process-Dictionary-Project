//
// Created by louis on 17/10/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "table.h"


void runNode(const int nodeID, int totalNode, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]){
    long res;
    unsigned char *frame;
    char *value = NULL;
    PAskFrame askFrame;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    Table_entry *dataBases = NULL;
    closePipes(pipeCtr,pipeRead,pipeWrite);
    do{
        frame = readFrame(pipeRead);
        askFrame = decodeAskFrame(frame);
        if(askFrame->cmd==0xff){
            DDP_perror("node.c::runNode() call decodeAskFrame()\n");
            exit(-1);
        }
        switch (askFrame->cmd) {
            case C_SET:
                if(askFrame->val%totalNode == nodeID){
                    char *value = malloc(sizeof(char)*4096);
                    printf("Saisir la valeur (chaine de characters):\n");
                    fgets(value, 4000, stdin);
                    if ((strlen(value) > 0) && (value[strlen (value) - 1] == '\n'))
                        value[strlen (value) - 1] = '\0';
                    store(&dataBases, askFrame->val, value);
                    acquittalFrame->cmd = A_SET;
                    acquittalFrame->nodeID = nodeID;
                    acquittalFrame->errorFlag=SUCCESS;
                    acquittalFrame->dataLength=0;
                    sendToController(acquittalFrame,pipeCtr);
                }else{
                    sendNextNode(frame,pipeWrite);
                }
                break;
            case C_LOOKUP:
                if(askFrame->val%totalNode == nodeID){
                    value = lookup(dataBases, askFrame->val);
                    if (value == NULL) {
                        acquittalFrame->cmd = A_LOOKUP;
                        acquittalFrame->nodeID = nodeID;
                        acquittalFrame->errorFlag= NOT_FOUND;
                        acquittalFrame->dataLength=0;
                        sendToController(acquittalFrame,pipeCtr);
                    }else{
                        unsigned int length = getCharLength((unsigned char *) value);
                        acquittalFrame->cmd = A_LOOKUP;
                        acquittalFrame->nodeID = nodeID;
                        acquittalFrame->errorFlag= SUCCESS;
                        acquittalFrame->dataLength= length;
                        acquittalFrame->data=malloc(sizeof(char) * length);
                        for(int i = 0; i<length;i++){
                            acquittalFrame->data[i]=value[i];
                        }
                        sendToController(acquittalFrame,pipeCtr);
                    }
                }else{
                    sendNextNode(frame,pipeWrite);
                }
                break;
            case C_DUMP:
                printf("dump du node %d:\n", nodeID);
                if(totalNode-1 != nodeID){
                    sendNextNode(frame,pipeWrite);
                }
                display(dataBases);
                acquittalFrame->cmd = A_DUMP;
                acquittalFrame->nodeID = nodeID;
                acquittalFrame->errorFlag=SUCCESS;
                acquittalFrame->dataLength=0;
                sendToController(acquittalFrame,pipeCtr);
                break;
            case C_EXIT:
                sendNextNode(frame,pipeWrite);
                break;
        }
    }while (askFrame->cmd!=C_EXIT);
    exit(0);
}

void closePipes(int * pipeCtr, int * pipeRead, int * pipeWrite){
    //Avant de commencer notre traitement on ferme les lecture/ecriture qui ne nous serve pas sur nos différent pipes
    //On ferme en lecture le pipe du controller
    close(pipeCtr[0]);
    //On ferme en ecriture le pipe de lecture
    close(pipeRead[1]);
    //On ferme en lecture le pipe d'écriture
    close(pipeWrite[0]);
}

unsigned char * readFrame(int pipeRead[2]){
    unsigned int nbBytes = 4096;
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

void sendToController(PAcquittalFrame acquittalFrame, int pipeCtr[2]){
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    long res;
    if(frame[0]==0xff){
        DDP_perror("node.c::runNode() call encodeAcquittalFrame()\n");
        exit(-1);
    }
    unsigned long len = strlen(( char *)frame);
    res = write(pipeCtr[1],frame,len);
    if (res == -1) {
        perror("node.c::runNode() call write() (To pipeCtr)\n");
        exit(-1);
    }
}

void sendNextNode(unsigned char * frame,int pipeWrite[2]) {
    unsigned int frameLength = strlen((char *) frame);
    if(write(pipeWrite[1],frame,frameLength)==-1){
        perror("node.c::sendNextNode() call write()");
        exit(-1);
    }
}

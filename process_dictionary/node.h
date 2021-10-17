//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_NODE_H
#define PROCESS_DICTIONARY_NODE_H

#include "DDP.h"

void runNode(int nodeID,int totalNode, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

int closePipes(int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

unsigned char * readFrame(int pipeRead[2]);

void sendToController(PAcquittalFrame acquittalFrame,int pipeCtr[2]);

void sendNextNode(unsigned char * frame,int pipeWrite[2]);

#endif //PROCESS_DICTIONARY_NODE_H

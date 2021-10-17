//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_NODE_H
#define PROCESS_DICTIONARY_NODE_H

#include "DDP.h"

void runNode(int nodeID, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

int closePipes(int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

unsigned char * readFrame(int pipeRead[2]);

int sendToController(PAcquittalFrame acquittalFrame,int pipeCtr[2]);

int sendNextNode(PAskFrame askFrame,int pipeWrite[2]);

#endif //PROCESS_DICTIONARY_NODE_H

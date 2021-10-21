//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_CONTROLLER_H
#define PROCESS_DICTIONARY_CONTROLLER_H

#include "DDP.h"

void runController(int nbNodes);

void initPipes(int nbNodes, int * pipeCtrl, int** pipeArr);

void launchPipes(int nbNodes, int * pipeCtrlRead, int** pipeArr);

void freeNodes(int nbNodes, int * pipeCtrl, int** pipeArr);

void cmdLauncher(int nbNodes,int * pipeCtrlWrite,int * pipeCtrlRead);

void askKey(unsigned char cmd,int * pipeCtrlWrite);

void readAcquittal(int *pipeCtrlRead);

int launchExit(int * pipeCtrlWrite);

int launchAsk(int * pipeCtrlWrite, unsigned char cmd, unsigned int val);

PAcquittalFrame deserialization(unsigned char * frameStream, unsigned int bufferSize);

void acquittalAction(PAcquittalFrame incomingAcquittal);

#endif //PROCESS_DICTIONARY_CONTROLLER_H

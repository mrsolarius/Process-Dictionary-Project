//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_CONTROLLER_H
#define PROCESS_DICTIONARY_CONTROLLER_H

#include "DDP.h"

int runController(int nbNodes);

void initPipes(int nbNodes, int * pipeCtrl, int** pipeArr);

void launchPipes(int nbNodes, int * pipeCtrlRead, int** pipeArr);

int freeNodes(int nbNodes, int * pipeCtrl, int** pipeArr);

int cmdLauncher(int nbNodes,int * pipeCtrlWrite,int * pipeCtrlRead);
int readAcquittal(int *pipeCtrlRead, int nbNodes);

int launchExit(int * pipeCtrlWrite);
int launchDump(int * pipeCtrlWrite,int nbNodes);
#endif //PROCESS_DICTIONARY_CONTROLLER_H

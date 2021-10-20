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
void askKey(unsigned char cmd,int * pipeCtrlWrite);

void readAcquittal(int *pipeCtrlRead);

int launchExit(int * pipeCtrlWrite);
int launchAsk(int * pipeCtrlWrite, unsigned char cmd, unsigned int val);
void print_hex(const unsigned char *s);
unsigned int getNbFrames(unsigned char * framesStream);

PAcquittalFrame * deserialisation(unsigned char * frameStream, unsigned int bufferSize, unsigned int *lenghtFrame);

unsigned char * substr(unsigned char *chaineSource,int pos,int len);

#endif //PROCESS_DICTIONARY_CONTROLLER_H

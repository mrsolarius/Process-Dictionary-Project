//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_CONTROLLER_H
#define PROCESS_DICTIONARY_CONTROLLER_H

#include "DDP.h"

int runController(int nbNodes);

int generateNodes(int nbNodes, int * pipeCtrl, int** pipeArr);

int freeNodes(int nbNodes, int * pipeCtrl, int** pipeArr);

#endif //PROCESS_DICTIONARY_CONTROLLER_H

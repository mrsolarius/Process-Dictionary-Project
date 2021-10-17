//
// Created by louis on 17/10/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "controller.h"
#include "node.h"

int runController(int nbNodes){
    //Allocation de la mémoire pour un tableau de nbNode * 2 car deux partie dans un pipe
    int **pipeArr = (int **) malloc(nbNodes * sizeof(int) * 2);
    //Allocation de la mémoire pour le pipe du controller
    int *pipeCtrlRead = (int *) malloc(2 * sizeof(int));
    generateNodes(nbNodes, pipeCtrlRead, pipeArr);
    int * pipeCtrlWrite = pipeArr[nbNodes - 1];
    close(pipeCtrlWrite[0]);
    //Injection de la premier valeur au node 0
    int val = 500;
    write(pipeCtrlWrite[1], &val, sizeof(int));

    int buffCtrl;
    //On lis les message envoyer par les fis
    while (read(pipeCtrlRead[0], &buffCtrl, sizeof(int)) > 0 && buffCtrl != nbNodes - 1){
        printf("Signal receive : %d\n", buffCtrl);
    }
    printf("Signal receive : %d\n", buffCtrl);
    freeNodes(nbNodes, pipeCtrlRead, pipeArr);
    return 0;
}

int generateNodes(int nbNodes, int * pipeCtrl, int ** pipeArr) {
    for (int i = 0; i < nbNodes; i++){
        //Allocation de la mémoire pour la ligne de la pipe courent
        pipeArr[i] = (int *) malloc(2 * sizeof(int));
        if(pipe(pipeArr[i])==-1){
            perror("controller.c::generateNodes() call pipe()");
            exit(-1);
        }
    }
    //On ouvre le pipe en direction du controller
    if(pipe(pipeCtrl)==-1){
        perror("controller.c::generateNodes() call pipe()");
        exit(-1);
    }

    for (int i = 0; i < nbNodes; i++) // Générer n node
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        if (fork() == 0) {
            if(i==0){
                runNode(i, pipeCtrl, pipeArr[nbNodes - 1], pipeArr[i]);
            }else{
                runNode(i,pipeCtrl,pipeArr[i-1],pipeArr[i]);
            }
        }
    }
    return 0;
}

int freeNodes(int nbNodes, int * pipeCtrl, int** pipeArr){
    //On libère du heap les tableaux et on attend les fis
    for (int i = 0; i < nbNodes; i++) {
        free(pipeArr[i]);
        wait(NULL);
    }
    free(pipeArr);
    free(pipeCtrl);
}
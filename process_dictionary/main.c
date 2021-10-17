//
// Created by louis on 13/10/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include "controller.h"

int main(int argc, char *argv[]){
    int nbNodes;

    if (argc != 2) {
        fprintf(stderr, "Argument insufisant\n");
        exit(-1);
    }
    nbNodes = atoi(argv[1]);
    if (nbNodes == -1) {
        fprintf(stderr, "Ce n'est pas un nb\n");
        exit(-1);
    }

    if(nbNodes < 2){
        fprintf(stderr, "Au moins 2 processus son requis\n");
        exit(-1);
    }
    runController(nbNodes);
    return 0;
}
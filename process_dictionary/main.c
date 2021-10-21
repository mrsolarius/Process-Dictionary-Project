#include <stdio.h>
#include <stdlib.h>
#include "controller.h"

int main(int argc, char *argv[]){
    int nbNodes;
    //verification qu'il y a bien un argument
    if (argc != 2) {
        fprintf(stderr, "Argument insufisant\n");
        exit(-1);
    }
    nbNodes = atoi(argv[1]);
    //Verification que l'argument envoyer est un nombre
    if (nbNodes == -1) {
        fprintf(stderr, "Ce n'est pas un nb\n");
        exit(-1);
    }
    //Verification que le nombre de nœuds est supérieur à 2
    if(nbNodes < 2){
        fprintf(stderr, "Au moins 2 processus son requis\n");
        exit(-1);
    }
    if(nbNodes>254){
        //si on dépace les 254 processus on retombe sur 0 car le numéro du processus et stoker sur un char + 1 (pour être bien compté dans les strlen)
        //Donc si on vient avec 0xff +1 pour le processus on tombe sur 0x00 donc on ne peut pas verifier que l'aquitement
        fprintf(stderr, "Pas plus de 254 processus\n");
        exit(-1);
    }
    //Lancement du programme
    runController(nbNodes);
    return 0;
}
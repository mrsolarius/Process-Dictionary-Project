#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "table.h"

//@todo refactor pour généraliser les fonctionnalité et améliorer la lisibilité du code

void runNode(const int nodeID, int totalNode, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]){
    unsigned char *frame;
    PAskFrame askFrame;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    //Definition de la BDD sera placer dans le HEAP des sont premier store
    Table_entry *dataBases = NULL;
    //On ferme les pipe aprés l'inisialisation
    closePipes(pipeCtr,pipeRead,pipeWrite);
    do{
        //programe principal du fis tant que une trame d'exit n'à pas était demander.

        //Lecture de la trame entrente
        frame = readFrame(pipeRead);
        askFrame = decodeAskFrame(frame);
        if(askFrame->cmd==0xff){
            DDP_perror("node.c::runNode() call decodeAskFrame()\n");
            exit(-1);
        }
        //Switch case du comportement à adopter en fonction des différente commande entrante
        switch (askFrame->cmd) {
            case C_SET:
                //Si l'id courent et egale à la valeur demander modulo le nombre de node on peut traiter
                //Sinon on envois notre trame d'entrer au prochain sibling
                if(askFrame->val%totalNode == nodeID){
                    int maxSize = 4000;
                    char *value = malloc(sizeof(char)*maxSize);
                    //on demande à l'utilisateur de saisir sa chaine de char
                    printf("Saisir la valeur (chaine de characters):\n");
                    fgets(value, maxSize, stdin);
                    //Stokage de la donnée dans la bdd
                    store(&dataBases, askFrame->val, value);
                    //Encodage d'une trame d'aquitement
                    acquittalFrame->cmd = A_SET;
                    acquittalFrame->nodeID = nodeID;
                    acquittalFrame->errorFlag=SUCCESS;
                    acquittalFrame->dataLength=0;
                    //Envoie de la trame d'aquittement au controller
                    sendToController(acquittalFrame,pipeCtr);
                }else{
                    //Si on peut pas traiter on envoie la trame renvoyer au prochain noeud
                    sendNextNode(frame,pipeWrite);
                }
                break;
            case C_LOOKUP:
                //Même principe que pour le c_set
                if(askFrame->val%totalNode == nodeID){
                    //On recupérer la valeur dans la BBD
                    char * value = lookup(dataBases, askFrame->val);
                    //Si on ne la trouve pas
                    if (value == NULL) {
                        //On générer et envoie une trame d'acquittement d'erreur au controller
                        acquittalFrame->cmd = A_LOOKUP;
                        acquittalFrame->nodeID = nodeID;
                        acquittalFrame->errorFlag= NOT_FOUND;
                        acquittalFrame->dataLength=0;
                        sendToController(acquittalFrame,pipeCtr);
                    }else{
                        //Sinon on générer et envoie une trame d'acquittement au controller contenant la valeur récupérer
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
                printf("dump du noeud %d:\n", nodeID);
                //tant que notre identifiant n'est pas egal au nombre total de neux -1 on envoie la trame de
                // dump au neud suivant
                if(totalNode-1 != nodeID){
                    sendNextNode(frame,pipeWrite);
                }
                //On affiche la BDD
                display(dataBases);
                //On envoie une trame d'aquitement au controller
                acquittalFrame->cmd = A_DUMP;
                acquittalFrame->nodeID = nodeID;
                acquittalFrame->errorFlag=SUCCESS;
                acquittalFrame->dataLength=0;
                sendToController(acquittalFrame,pipeCtr);
                break;
            case C_EXIT:
                //On envoie le exit au prochain neud
                //Comme on et sur une trame d'exit on quitera automatiquement la boucle
                sendNextNode(frame,pipeWrite);
                break;
        }
    }while (askFrame->cmd!=C_EXIT);
    //puis le programme
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
    //malloc de la trame dans la mémoire
    unsigned char * frame = malloc(sizeof (char)*nbBytes);
    long res=1;
    // lecture du pipe d'entrer et renvoie de celui ci
    res = read(pipeRead[0], frame, nbBytes);
    if (res == -1) {
        perror("node.c::runNode() call read()");
        exit(-1);
    }
    return frame;
}

void sendToController(PAcquittalFrame acquittalFrame, int pipeCtr[2]){
    // encodage de la trame
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    long res;
    if(frame[0]==0xff){
        DDP_perror("node.c::runNode() call encodeAcquittalFrame()\n");
        exit(-1);
    }
    unsigned long len = strlen(( char *)frame);
    //envoie d'une payload au controller si le pipe en parameter correctement renseigner
    res = write(pipeCtr[1],frame,len);
    if (res == -1) {
        perror("node.c::runNode() call write() (To pipeCtr)\n");
        exit(-1);
    }
}

void sendNextNode(unsigned char * frame,int pipeWrite[2]) {
    unsigned int frameLength = strlen((char *) frame);
    //envoie d'une payload au nœud suivant si le pipe en parameter correctement renseigner
    if(write(pipeWrite[1],frame,frameLength)==-1){
        perror("node.c::sendNextNode() call write()");
        exit(-1);
    }
}

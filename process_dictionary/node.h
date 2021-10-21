//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_NODE_H
#define PROCESS_DICTIONARY_NODE_H

#include "DDP.h"

/**
 * Nom :
 *      runNode - Permet de lancer un noeud enfant
 * Description :
 *      Cette fonction contient une boucle while qui lit les trames entrante. La condition d'arrêt et de recevoir
 *      une trame EXIT.
 *
 *      Cette fonction correspond au programme principal du fils
 *      Elle gère aussi l'interface homme machine.
 * Paramètres :
 *      @param nodeID correspond à l'identifiant unique donné par le contrôleur au fils
 *      @param totalNode correspond au nombre total de noeud générer par le contrôleur
 *      @param pipeCtr correspond au pipe du contrôleur
 *      @param pipeRead correspond au pipe de lecture du noeud
 *      @param pipeWrite correspond au pipe d'écriture du noeud
 * Erreurs
 *      Cette fonction peut générer 1 erreur une erreur de DDP sur le decodage de la trame ASK
 */
void runNode(int nodeID,int totalNode, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

/**
 * Nom :
 *      closePipes - Cette fonction permet de fermer tous les pipe non utile au fils
 * Description :
 *      Comme le fils n'utilise que la partie écriture ou lecture des ces pipe on vas fermer la partie qu'il
 *      n'utilise pas
 * Paramètres
 *      @param pipeCtr correspond au pipe du contrôleur
 *      @param pipeRead correspond au pipe de lecture du noeud
 *      @param pipeWrite correspond au pipe d'écriture du noeud
 */
void closePipes(int pipeCtr[2], int pipeRead[2], int pipeWrite[2]);

unsigned char * readFrame(int pipeRead[2]);

void sendToController(PAcquittalFrame acquittalFrame,int pipeCtr[2]);

void sendNextNode(unsigned char * frame,int pipeWrite[2]);

#endif //PROCESS_DICTIONARY_NODE_H

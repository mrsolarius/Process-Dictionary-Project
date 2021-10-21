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

/**
 * Nom :
 *      readFrame - permet de lire le contenue du pipe passer en parameter et le renvoyer sous fourme d'un tableau de
 *      unsigned char
 * Paramètres :
 *      @param pipeRead pipe de lecture du node
 * Retour :
 *      @return un tableau de unsigned char *
 */
unsigned char * readFrame(int pipeRead[2]);

/**
 * Nom :
 *      sendToController - permet d'envoyer au pipe passer en parameter la structure acquittal de trame.
 * Paramètres :
 *      @param acquittalFrame correspond à la structure d'une trame d'acquittement destinée au contrôleur
 *      @param pipeCtr pipe destiner au contrôleur
 * Erreurs :
 *      Peut renvoyer deux erreur : une erreur de type DDP et une erreur d'écriture dans le pipe du contrôleur
 */
void sendToController(PAcquittalFrame acquittalFrame,int pipeCtr[2]);

/**
 * Nom :
 *      sendNextNode - permet d'envoyer au pipe passer en parameter la trame indiquer en paramètres
 * Description :
 *      Cette fonction permet de renvoyer la trame reçu du dernier noeud vers le noeud suivant
 * Paramètres :
 *      @param frame correspond au tableau de unsigned char à envoyer
 *      @param pipeWrite correspond au pipe d'écriture destiner au noeud suivant
 * Erreurs :
 *      Peur renvoyer une erreur d'écriture
 */
void sendNextNode(unsigned char * frame,int pipeWrite[2]);

#endif //PROCESS_DICTIONARY_NODE_H

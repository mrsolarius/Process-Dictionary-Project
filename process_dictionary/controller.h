//
// Created by louis on 17/10/2021.
//

#ifndef PROCESS_DICTIONARY_CONTROLLER_H
#define PROCESS_DICTIONARY_CONTROLLER_H

#include "DDP.h"

/**
 * Variable global permettant de savoir le nombre de trame d'acquittement lu depuis le précédent reset
 * Elle est utiliser notamment pour attendre toutes les trame d'acquittement d'un dump.
 */
extern unsigned int countAcquittal;
/**
 * Nom :
 *      runController - Permet de lancer l'application avec un nombre de processus fils definies
 * Description :
 *     la fonction vas demande la génération des pipe entre tous les processus fis, puis elle vas
 *     demander le lancement de neud enfant. Une fois fait elle vas lancer la fonction d'interface
 *     utilisateur permettant l'interaction avec l'application.
 *
 *     Lorsque l'interaction avec l'utilisateur est terminé alors la  fonction demande la fermeture des pipe du controler
 *     puis la fermeture et la liberation la liberation de tous les pipe
 * Paramètre :
 *      @param nbNodes correspond au nombre de processus fils demander par l'utilisateur.
 */
void runController(int nbNodes);

/**
 * Nom :
 *      initPipes - Permet d'initialiser un tableau de pipe pour un nombre de noeud defines et un pipe simple
 * Description :
 *      Cette fonction ouvre les descripteur de fichier sur les tableau de pipe et les pipe passer en parametre
 * Paramètres :
 *      @param nbNodes correspond au nombre de pipe qu'il faut initialiser dans le tableau de pipe
 *      @param pipeCtrl correspond as l'adresse du pipe simple du controller qu'il faut initialiser
 *      @param pipeArr correspond as l'adresse du tableau de pipe qu'il faut initialiser
 * Erreurs :
 *      Peut avoir une erreur sur l'ouverture de pipes et affichera un perror() en conséquence
 */
void initPipes(int nbNodes, int * pipeCtrl, int** pipeArr);

/**
 * Nom:
 *      launchPipes - Permet de lancer les processus enfants
 * Description :
 *      la fonction va générer autant de processus enfant qu'indiquer avec les paramètres dont les enfants on besoin
 * Parameters :
 *      @param nbNodes correspond au nombre de noeud enfant à crée
 *      @param pipeCtrl correspond au pipe de communication avec le controller
 *      @param pipeArr correspond au pipe initialiser des lecture et d'écritures de chaque enfant
 * Erreurs :
 *      Peut renvoyer une erreur sur le fork et affichera un perror() en conséquence
 */
void launchPipes(int nbNodes, int * pipeCtrl, int** pipeArr);

/**
 * Nom:
 *      freeNodes - Permet de ferment les pipes, d'attendre la fin d'execution des fis, et de libérer la mémoire du HEAP
 * Description :
 *      Cette fonction permet de terminer le programme en libérant toutes les ressources que celui prend correctement.
 * Parameters :
 *      @param nbNodes correspond au nombre de noeud enfant à crée
 *      @param pipeCtrl correspond au pipe de communication avec le controller
 *      @param pipeArr correspond au pipe initialiser des lecture et d'écritures de chaque enfant
 */
void freeNodes(int nbNodes, int * pipeCtrl, int** pipeArr);

/**
 * Nom :
 *      cmdLauncher - Correspond à la boucle principal du programme. Elle reste active tant que l'utilisateur ne la quitte pas
 *      avec la commande exit
 *  Description :
 *      Cette fonction permet la selection des trame à envoyer au processus fils dans le but de faire une action.
 *      On demande à l'utilisateur de lancer une commande qui sera execute par l'un des processus fils.
 *  Parameters :
 *      @param nbNodes correspond au nombre de noeud enfant à crée
 *      @param pipeCtrlWrite correspond au pipe d'écriture du controller
 *      @param pipeCtrlRead correspond au pipe de lecture du controller
 */
void cmdLauncher(int nbNodes,int * pipeCtrlWrite,int * pipeCtrlRead);

/**
 * Nom :
 *      askKey - Demande à l'utilisateur de saisir la clef vers la quelle une commande précisée en paramètre sera envoyer
 * Description :
 *      cette fonction ne fait que de demander une clef puis appel la fonction launchAsk()
 * Parameters :
 *      @param cmd correspond à la commande à envoyer
 *      @param pipeCtrlWrite correspond au pipe d'écriture du controller
 */
void askKey(unsigned char cmd,int * pipeCtrlWrite);

/**
 * Nom :
 *      readAcquittal - Permet de lire les trame en provenance des enfants via le pipe de lecture du contrôleur
 * Description :
 *      Cette fonction lis dans le pipe du controller afin de récupérer les donnée entrante puis va demander la
 *      désérialisassions des données entrante en faisant appel à la fonction deserialization() puis va les envoyés
 *      à la fonction acquittalAction afin de les traiter convenablement.
 * Parameters :
 *      @param pipeCtrlRead correspond au pipe de lecture du controller
 * Erreurs :
 *      Peut renvoyer deux erreurs différentes une erreur de lecture sur le pipe et une erreur de lecture de la trame
 */
void readAcquittal(int *pipeCtrlRead);

/**
 * Nom :
 *      launchExit - Permet d'envoyer une trame d'exit au premier node
 * Parameters :
 *      @param pipeCtrlWrite correspond au pipe d'écriture du controller
 * Return :
 *      @return renvoie 0 si tous c'est bien passer
 * Erreurs :
 *      Peut renvoyer deux erreurs différentes une erreur de d'écriture sur le pipe et une erreur de lecture de la trame
 */
int launchExit(int * pipeCtrlWrite);

/**
 * Nom :
 *      launchAsk - permet d'envoyer une trame de demande au premier node
 * Description :
 *      Cette fonction encode une trame de demande (ASK) à partir d'une commande et d'une valeur positive
 * Parameters :
 *      @param pipeCtrlWrite correspond au pipe d'écriture du controller
 *      @param cmd correspond à la commande ask à envoyer
 *      @param val correspond à la clef à envoyer
 * Return :
 *      @return renvoie 0 si tous c'est bien passer
 * Erreurs :
 *      Peut renvoyer deux erreurs différentes une erreur de d'écriture sur le pipe et une erreur de lecture de la trame
 */
int launchAsk(int * pipeCtrlWrite, unsigned char cmd, unsigned int val);

/**
 * Nom :
 *      deserialization - Permet de décoder un stream de plusieurs trame en un tableau de PAcquittalFrame (ne le fait pas encore)
 * Description :
 *      Coupe la trame d'entrée en plusieurs PAcquittalFrame de sortie elle est capable de compter le nombre de trame écrites
 *      dans un pipe si le pipe à reçu plusieurs trame en même temps.
 *
 *      Actuellement elle ne renvoie que la première trame trouvée.
 *
 *      Elle incrémente la variable global countAcquittal à chaque fois que elle trouve une trame bien formé dans le stream.
 *      La fonction peut recevoir plusieurs trame dans le stream car tous les fils peuvent écrire en même temps dans le pipe
 *      du contrôleur.
 * Parameters
 *      @param frameStream correspond à au tableau de unsigned char lu dans le pipe
 *      @param bufferSize correspond à la taille du buffer
 * Return :
 *      @return Renvoie la structure de la première trame trouvée dans le flux
 */
PAcquittalFrame deserialization(unsigned char * frameStream, unsigned int bufferSize);

/**
 *  Nom :
 *      acquittalAction - Permet de réagir aux d'acquittement trames reçu par le controller
 *  Description :
 *      En fonction des trame reçus on affichera les donnée dans la console sous une forme différent
 *  Parameters
 *      @param incomingAcquittal correspond au pointeur d'une trame d'acquittement
 */
void acquittalAction(PAcquittalFrame incomingAcquittal);

#endif //PROCESS_DICTIONARY_CONTROLLER_H

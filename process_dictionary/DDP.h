//
// Created by louis on 12/10/2021.
//

#ifndef PROCESS_DICTIONARY_PROJECT_DDP_H
#define PROCESS_DICTIONARY_PROJECT_DDP_H

/**
 * Définis la fin d'une trame
 */
const unsigned char END_FRAME = 0x04;

/**
 * Toutes les commandes de demande de DDP
 */
const unsigned char C_SET = 0xA1;
const unsigned char C_LOOKUP = 0xB1;
const unsigned char C_DUMP = 0xD1;
const unsigned char C_EXIT = 0xE1;

/**
 * Toutes les commandes d'acquittement de DDP
 */
const unsigned char A_SET = 0xA2;
const unsigned char A_LOOKUP = 0xB2;
const unsigned char A_DUMP = 0xD2;

/**
 * Tous les drapeau d'erreur de DDP
 */
const unsigned char SUCCESS = 0x20;
const unsigned char NOT_FOUND = 0x44;
const unsigned char INTERNAL_ERROR = 0x50;

/**
 * Definition du type de la trame de demande
 */
typedef struct _AskFrame {
    unsigned char cmd;
    unsigned int val;
} AskFrame;

typedef AskFrame *PAskFrame;

/**
 * Definition du type de la trame d'acquittement
 */
typedef struct _AcquittalFrame {
    unsigned char cmd;
    unsigned char nodeID;
    unsigned char errorFlag;
    unsigned int dataLength;
    char *data;
} AcquittalFrame;

typedef AcquittalFrame *PAcquittalFrame;

/**
 * Nom :
 *      evaluateFrame - Permet d'evaluer le type de la trame DDP passer en paramètre sous forme de char
 * Description :
 *      evaluateFrame() vas tenter de savoir si la chaine de character envoyer est une trame de DDP pouvant être utiliser
 * Paramètre :
 *      @param frame correspond au buffer d'une trame de DDP
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvera :
 *          0x01 si c'est une trame de type ASK
 *          0x02 si c'est une trame de type ACQUITTAL
 *
 *      Si l'operation est un échèque la fonction renvera -1 et errno sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTDDP     Là chaine de character ne correspond pas au protocole DDP
 *      EBADCMD     Là commande demander ne correspond pas à la trame identifier
 *      EBADNODE    L'identififant du node est invalide
 *      EWRONGEFLAG Le drapeau indique une erreur mais une data est présente
 *      EWRONGSFLAG Le drapeau indique un succes mais aucune data est présente
 *      EWRONGLEN   La longueur de data indique ne corespond pas à celle de la data
 *      NOTENDFLAG  Aucun drapeau de fin de trame n'a était identifier
 */
char evaluateFrame(char *frame);

/**
 * Nom :
 *      decodeAskFrame - Permet de décoder une trame de DDP passer en paramètre sous forme de char
 * Description :
 *      decodeAskFrame() vas tenter de décoder une trame de type ASK pour la renvoyer sous la forme de PAskFrame
 * Paramètre :
 *      @param frame correspond au buffer d'une trame de DDP
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvois le pointer vers la trame parser de type AskFrame
 *
 *      Si l'opération est un echec la fonction renverra une valeurs NULL et errno sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTASK Là trame envoyer n'est pas une trame de type ASK
 *
 *      les erreur de evaluateFrame() peuvent êtres renvoyer des erreur ici
 */
PAskFrame decodeAskFrame(char *frame);

/**
 * Nom :
 *      decodeAcquittalFrame - Permet de décoder une trame de DDP passer en paramètre sous forme de char
 * Description :
 *      decodeAcquittalFrame() vas tenter de décoder une trame de type ACQUITTAL pour la renvoyer sous la forme de PAcquittalFrame
 * Paramètre :
 *      @param frame correspond au buffer d'une trame de DDP
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvois le pointer vers la trame parser de type AskFrame
 *
 *      Si l'opération est un echec la fonction renverra une valeurs NULL et errno sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTAQIT Là trame envoyer n'est pas une trame de type ACQUITTAL
 *
 *      les erreur de evaluateFrame() peuvent êtres renvoyer des erreur ici
 */
PAcquittalFrame decodeAcquittalFrame(char *frame);

/**
 * Nom :
 *      encodeAskFrame - Permet d'encoder un PAskFrame sous forme d'une ASK FRAME
 * Description :
 *      encodeAskFrame() vas sérialiser une PAskFrame sous forme d'une ASK FRAME
 * Paramètre :
 *      @param askFrame correspond a la structure d'un ASK FRAME
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvois une chaine de character correspondent
 *      à la sérialisation d'une ASK FRAME
 *
 *      Si l'opération est un echec la fonction renverra -1 et errno sera definie et positioner de manière approprier
 * Erreurs :
 *      EBADCMD Là commande demander ne correspond pas à la trame identifier
 */
char *encodeAskFrame(PAskFrame askFrame);

/**
 * Nom :
 *      encodeAcquittalFrame - Permet d'encoder un PAcquittalFrame sous forme d'une ACQUITTAL FRAME
 * Description :
 *      encodeAcquittalFrame() vas sérialiser une PAskFrame sous forme d'une ACQUITTAL FRAME
 * Paramètre :
 *      @param acquittalFrame correspond a la structure d'un ACQUITTAL FRAME
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvois une chaine de character correspondent
 *      à la sérialisation d'une ACQUITTAL FRAME
 *
 *      Si l'opération est un echec la fonction renverra -1 et errno sera definie et positioner de manière approprier
 * Erreurs :
 *      EBADCMD     Là commande demander ne correspond pas à la trame identifier
 *      EBADNODE    L'identififant du node est invalide
 *      EWRONGEFLAG Le drapeau indique une erreur mais une data est présente
 *      EWRONGSFLAG Le drapeau indique un succes mais aucune data est présente
 *      EWRONGLEN   La longueur de data indique ne correspond pas à celle de la data
 */
char *encodeAcquittalFrame(PAcquittalFrame acquittalFrame);

#endif //PROCESS_DICTIONARY_PROJECT_DDP_H

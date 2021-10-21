//
// Created by louis on 12/10/2021.
//

#ifndef PROCESS_DICTIONARY_PROJECT_DDP_H
#define PROCESS_DICTIONARY_PROJECT_DDP_H

/**
 * Equivalent grossier de errno pour récupérer les erreurs interne de DDP
 */
extern int DDP_Errno;

/**
 * Enum de toutes les erreurs
 */
enum DDP_errCode {
    ENOTDDP=0x0,
    EBADCMD=0x1,
    EWRONGEFLAG=0x2,
    EWRONGSFLAG=0x3,
    EWRONGNFLAG=0x4,
    EWRONGLEN=0x5,
    ENOENDFLAG=0x6,
    ENOTASK=0x7,
    ENOTAQIT=0x8,
    ENOTNULL=0x9
};

extern const char * DDP_errList[];

/**
 * Enum des types de frame
 */
enum frameType{
    ASK=1,
    ACQUITTAL=2
};

/**
 * Enum de toutes les frames
 */
enum frameComponents {
    /**
     * Définis la fin d'une trame
     */
    END_FRAME=(unsigned char)0x04,


    /**
     * Toutes les commandes de demande de DDP
     */
    C_SET=(unsigned char)0xA1,
    C_LOOKUP=(unsigned char)0xB1,
    C_DUMP=(unsigned char)0xD1,
    C_EXIT=(unsigned char)0xE1,

    /**
     * Toutes les commandes d'acquittement de DDP
     */
    A_SET=(unsigned char)0xA2,
    A_LOOKUP=(unsigned char)0xB2,
    A_DUMP=(unsigned char)0xD2,

    /**
     * Tous les drapeau d'erreur de DDP
     */
    SUCCESS=(unsigned char)0x20,
    NOT_FOUND=(unsigned char)0x44,
    INTERNAL_ERROR=(unsigned char)0x50
};

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
    unsigned short dataLength;
    unsigned char *data;
} AcquittalFrame;

typedef AcquittalFrame *PAcquittalFrame;

/**
 * Nom :
 *      evaluateType - Permet d'evaluer le type de la trame DDP passer en paramètre sous forme de char
 * Description :
 *      evaluateType() vas tenter de savoir si la chaine de character envoyer est une trame de DDP pouvant être utiliser
 * Paramètre :
 *      @param frame correspond au buffer d'une trame de DDP
 * Valeur de retour :
 *      Si l'operation est un succès la fonction renvera :
 *          0x01 si c'est une trame de type ASK
 *          0x02 si c'est une trame de type ACQUITTAL
 *
 *      Si l'operation est un échèque la fonction renvera -1 et DDPperror sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTDDP Là chaine de character ne correspond pas au protocole DDP
 */
char evaluateType(const unsigned char frame);

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
 *      Si l'opération est un echec la fonction renverra une valeurs 0xff dans la partie cmd de la structure
 *      et DDPperror sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTASK    Là trame envoyer n'est pas une trame de type ASK
 *      EBADCMD    Là commande demander ne correspond pas à la trame identifier
 *      ENOENDFLAG Aucun drapeau de fin de trame n'a était identifier
 *      ENOTDDP    Là chaine de character ne correspond pas au protocole DDP
 */
PAskFrame decodeAskFrame(unsigned char *frame);

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
 *      Si l'opération est un echec la fonction renverra une valeurs 0xff dans la partie cmd de la structure
 *      et DDPperror sera definie et positioner de manière approprier
 * Erreurs :
 *      ENOTAQIT    Là trame envoyer n'est pas une trame de type ACQUITTAL
 *      EBADCMD     Là commande demander ne correspond pas à la trame identifier
 *      EWRONGEFLAG Le drapeau indique une erreur mais une data est présente
 *      EWRONGSFLAG Le drapeau indique un succes mais aucune data est présente
 *      EWRONGNFLAG Le drapeau indique not found mais c'est impossible sur un SET
 *      EWRONGLEN   La longueur de data indique ne corespond pas à celle de la data
 *      ENOENDFLAG  Aucun drapeau de fin de trame n'a était identifier
 *      ENOTDDP     Là chaine de character ne correspond pas au protocole DDP
 */
PAcquittalFrame decodeAcquittalFrame(const unsigned char *frame, unsigned int frameLength);

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
 *      Si l'opération est un echec la fonction renverra 0xff dans la premier case du tableau
 *      et DDPperror sera definie et positioner de manière approprier
 * Erreurs :
 *      EBADCMD Là commande demander ne correspond pas à la trame identifier
 *      ENOTDDP Là chaine de character ne correspond pas au protocole DDP
 */
unsigned char * encodeAskFrame(PAskFrame askFrame);

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
 *      Si l'opération est un echec la fonction renverra 0xff dans la premier case du tableau
 *      et DDPperror sera definie et positioner de manière approprier
 * Erreurs :
 *      EBADCMD     Là commande demander ne correspond pas à la trame identifier
 *      EBADNODE    L'identififant du node est invalide
 *      EWRONGEFLAG Le drapeau indique une erreur mais une data est présente
 *      EWRONGSFLAG Le drapeau indique un succes mais aucune data est présente
 *      EWRONGLEN   La longueur de data indique ne correspond pas à celle de la data
 */
unsigned char * encodeAcquittalFrame(PAcquittalFrame acquittalFrame);

/**
 * Nom :
 *      DDP_perror - Afficher un message d'erreur de DDP
 * Description :
 *      La fonction DDP_perror() affiche un message sur la sortie d'erreur standard, décrivant la dernière erreur rencontrée lors de l'utilisation de DDP.
 *      D'abord, (si s n'est pas NULL et si *s n'est pas un octet nul), la chaîne de caractère s est imprimée, suivie de deux points (« : »),
 *      puis le message, suivi d'un saut de ligne.
 *
 *      La chaîne de caractères contient généralement le nom de la fonction où s'est produit l'erreur.
 *      Le numéro d'erreur est obtenu à partir de la variable externe DDP_Errno, qui contient le code d'erreur lorsqu'un problème survient,
 *      mais qui s'est pas effacée lorsqu'un appel est réussi.
 *
 *      La liste globale d'erreurs DDP_errList[] indexée par DDP_errno peut être utilisée pour obtenir le message d'erreur sans DDP_perror.
 *
 * Paramètre :
 *      @param msg correspond a message précédent l'erreur
 */
void DDP_perror(char* msg);

unsigned char getCharLength(unsigned char * val);

#endif //PROCESS_DICTIONARY_PROJECT_DDP_H

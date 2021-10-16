//
// Created by louis on 16/10/2021.
//

#include "DDP.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

/*****************************
 *  Fonction utile au tests  *
 *****************************/

int testPassed = 0;
int totalTest = 0;

void printTitle(char *title) {
    unsigned int size = strlen(title);
    printf(COLOR_BLUE"\n+");
    for (int i = 0; i < size + 4; i++) {
        printf("-");
    }
    printf("+\n|"COLOR_BLUE COLOR_YELLOW"  %s  "COLOR_BLUE COLOR_BLUE"|\n+", title);
    for (int i = 0; i < size + 4; i++) {
        printf("-");
    }
    printf("+\n"COLOR_RESET);
}

void passTest(char *testFunction, char *msg, bool res) {
    totalTest++;
    printf(COLOR_BLUE"%s : %s\n"COLOR_RESET, testFunction, msg);
    if (res) {
        printf(COLOR_GREEN"\t✔️ test passed\n"COLOR_RESET);
        testPassed++;
    } else {
        printf(COLOR_RED"\t❌ test failure\n"COLOR_RESET);
    }
}

void resume() {
    printf(COLOR_YELLOW"\n%d teste réussie sur %d (%d/%d)\n"COLOR_RESET, testPassed, totalTest, testPassed, totalTest);
}

/*****************************
 *  test de evaluateType()  *
 ****************************/
bool evaluateType_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    char val = evaluateType(0xff);
    //La commande 0xff n'existe pas dans le protocole DDP cela doit donc renvoyer l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (val == -1);
}

bool evaluateType_itShouldBeAskFrame() {
    char frame = evaluateType(C_SET);
    //La commande C_SET est de type ASK la fonction doit donc renvoyer ASK(0x1)
    return frame == ASK;
}

bool evaluateType_itShouldBeAcquittalFrame() {
    char frame = evaluateType(A_SET);
    //La commande A_SET est de type ACQUITTAL la fonction doit donc renvoyer ACQUITTAL(0x2)
    return frame == ACQUITTAL;
}

/******************************
 *  test de decodeAskFrame()  *
 ******************************/
bool decodeAskFrame_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 'n', 'o', 't', ' ', 'a', ' ', 'f', 'r', 'a', 'm'};
    PAskFrame res = decodeAskFrame(frame);
    //La trame envoyer commence par une commande mais la suite n'existe pas donc ce n'est pas une trame du
    //protocole DDP on renvoie l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOTASK() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_SET, 2, SUCCESS, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    //La trame envoyer et correct cependant la commande A_SET n'est pas une commande de type ASK
    //il faut donc renvoyer l'erreur ENOTASK
    return (DDP_Errno == ENOTASK) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowEBADCMD() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    //La commande C_SET doit obligatoirement avoir une valeur donc on renvoie une erreur EBADCMD
    //Car cela devrais plutôt être la commande C_EXIT qui devrais être envoyer
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOENDFLAG() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_EXIT, 2};
    PAskFrame res = decodeAskFrame(frame);
    //Ici la derniere valeur du tableau devrais être END_FRAME or ici ce n'est que 2
    //On s'attend donc à recevoir l'erreur NO END FLAG
    return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowEBADCMD_2() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_EXIT, 0xA, 0xA, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    //La commande EXIT ne peut pas avoir de valeur donc on renvoie une erreur EBADCMD
    //Car cela devrais plutôt être les commandes C_SET C_LOOKUP ou C_DUMP qui devrais être envoyer
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOENDFLAG_2() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 0xA, 0xA, 0xFF};
    PAskFrame res = decodeAskFrame(frame);
    //Ici la derniere valeur du tableau devrais être END_FRAME or ici ce n'est que 2
    //On s'attend donc à recevoir l'erreur NO END FLAG
    return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldCorrectlyDecodeThisFrame() {
    DDP_Errno = -1;
     //On veut envoyer la valeur 125 soit 0b0000 0000 0111 1101
     //on envoie donc 0 0x7D
    unsigned char frame[] = {C_SET, 0 + 1, 0x7D + 1, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    //Ici la frame et bien former et devrais donc renvoyer les bonnes valeurs
    return (res->cmd == C_SET) && (res->val == 125);
}

bool decodeAskFrame_itShouldCorrectlyDecodeThisFrame_2() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_EXIT, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    //Ici aussi la frame et bien former
    return (res->cmd == C_EXIT);
}

/************************************
 *  test de decodeAcquittalFrame()  *
 ************************************/
bool decodeAcquittalFrame_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    //permet de tester pour une taille non détérminer supérieur à 4
    unsigned char frame[] = {A_LOOKUP, 'n', 'o', 't', ' ', 'a', ' ', 'f', 'r', 'a', 'm'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //La trame envoyer commence par une commande mais la suite n'existe pas donc ce n'est pas une trame du
    //protocole DDP on renvoie l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_2() {
    DDP_Errno = -1;
    //Permet de tester pour une taille de 4 pille
    unsigned char frame[] = {A_SET, 'n', 'o', 't'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //La trame envoyer commence par une commande mais la suite n'existe pas donc ce n'est pas une trame du
    //protocole DDP on renvoie l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_3() {
    DDP_Errno = -1;
    //Permet de tester pour une taille inférieur à 4
    unsigned char frame[] = {A_SET, 'n', 'o'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //La trame envoyer commence par une commande mais la suite n'existe pas donc ce n'est pas une trame du
    //protocole DDP on renvoie l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_4() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 1, 0x55, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //La trame utilise un drapeau inconue par le protocole DDP on renvoie l'erreur ENOTDDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOTAQIT() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 250, SUCCESS, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //La trame envoyer et correct cependant la commande C_SET n'est pas une commande de type ACQUITTAL
    //il faut donc renvoyer l'erreur ENOTAQIT
    return (DDP_Errno == ENOTAQIT) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ASET() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_SET, 1, NOT_FOUND, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Le flag d'erreur not found n'est pas possible pour la trame A_SET
    //Il faut donc renvoyer WRONG NOT FOUND FLAG
    return (DDP_Errno == EWRONGNFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ADUMP() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_DUMP, 1, NOT_FOUND, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Le flag d'erreur not found n'est pas possible pour la trame A_DUMP
    //Il faut donc renvoyer WRONG NOT FOUND FLAG
    return (DDP_Errno == EWRONGNFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGSFLAG_ALOOKUP() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 1, INTERNAL_ERROR, 0x0+1,0x2+1,'h','i', END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Le flag d'erreur INTERNAL_ERROR ou NOT_FOUND n'est pas possible pour la trame A_LOOKUP contenant des données
    //Il faut donc renvoyer WRONG ERROR FLAG
    return (DDP_Errno == EWRONGEFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGEFLAG_ALOOKUP() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 1, SUCCESS, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Le flag de success n'est pas autoriser si aucune donné lui sont associer pour un A_LOOKUP
    //Il faut donc renvoyer WRONG SUCCESS FLAG
    return (DDP_Errno == EWRONGSFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, INTERNAL_ERROR, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Ici la trame et bien composer tous devrais fonctioner
    return (
                   (
                           (res->cmd == A_LOOKUP) &&
                           (res->nodeID == 0)
                   ) && (
                           (res->errorFlag == INTERNAL_ERROR) &&
                           (res->dataLength == 0)
                   )
           ) &&
           (res->data == NULL);
}

bool decodeAcquittalFrame_itShouldThrowEBADCMD(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_DUMP, 0 + 1, SUCCESS, 0x0+1,0x2+1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Un A_DUMP ne peut pas renvoyer de donnée c'est un mauvais choix de donné
    //Il faut renvoyer une erreur de BAD COMMANDE
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOTDDPError_4(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, 0x55, 0x0+1,0x2+1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Le flag 0x55 n'existe pas dans DDP il faut donc renvoyer l'erreur NOT DDP
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGEFLAG(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, NOT_FOUND, 0x0+1,0x2+1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Ici la derniere valeur du tableau devrais être END_FRAME or ici ce n'est que 0x55
    //On s'attend donc à recevoir l'erreur NO END FLAG
    return (DDP_Errno == EWRONGEFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOENDFLAG_2(){
        DDP_Errno = -1;
        unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0+1,0x2+1,'h','i',0xEE};
        PAcquittalFrame res = decodeAcquittalFrame(frame);
        //Ici la derniere valeur du tableau devrais être END_FRAME or ici ce n'est que 0xEE
        //On s'attend donc à recevoir l'erreur NO END FLAG
        return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGLEN(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0 + 1,0x3 + 1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Ici la taille indiquer par la partie data length et de 3 or la longeurs du tableau de donnée ici est de 2
    //Il faut donc renvoyer une erreur de WRONG LENGTH
    return (DDP_Errno == EWRONGLEN) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame_2(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0 + 1,0x2 + 1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    //Ici la trame et bien composer tous devrais fonctioner
    return (
                   (
                           (res->cmd == A_LOOKUP) &&
                           (res->nodeID == 0)
                   ) && (
                           (res->errorFlag == SUCCESS) &&
                           (res->dataLength == 2)
                   )
           ) &&
           (memcmp(res->data,(unsigned char *)"hi",2) == 0);
}


/*******************************
 *  test de encodeAskFrame()  *
 ******************************/

bool encodeAskFrame_itShouldThrowEBADCMD(){
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=A_SET;
    askFrame->val=2;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    //A_SET n'est pas une commande destiner à être utiliser par la trame ASK
    //Il faut renvoyer une erreur de BAD COMMANDE
    return (DDP_Errno == EBADCMD) && (data[0] == 0xff);
}

bool encodeAskFrame_itShouldThrowENOTDDP(){
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=0x51;
    askFrame->val=2;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    //0x51 n'est pas une commande de DDP
    //Il faut renvoyer une erreur de NOT DDP
    return (DDP_Errno == ENOTDDP) && (data[0] == 0xff);
}

bool encodeAskFrame_itShouldEncodeThisFrame(){
    //25 = 0b 0000 0000 0001 1001
    //                   0x1  0x9
    //          0x00       0x19
    unsigned char expectedFrame[] = {C_LOOKUP, 0x0 + 1, 0x19 + 1, END_FRAME};
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=C_LOOKUP;
    askFrame->val=25;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    //ici la trame et bien former on s'attend donc à ce que sa correspondre à la expectedFrame
    return memcmp(expectedFrame,data,4)==0;
}

bool encodeAskFrame_itShouldEncodeThisFrame_2(){
    unsigned char expectedFrame[] = {C_EXIT, END_FRAME};
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=C_EXIT;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    //ici la trame et bien former on s'attend donc à ce que sa correspondre à la expectedFrame
    return memcmp(expectedFrame,data,2)==0;
}

/************************************
 *  test de encodeAcquittalFrame()  *
 ************************************/
bool encodeAcquittalFrame_itShouldThrowEBADCMD(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=C_SET;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=SUCCESS;
    acquittalFrame->dataLength=0;
    unsigned char * data = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //C_SET n'est pas une commande destiner à être utilisé par la trame ACQUITTAL
    //Il faut renvoyer une erreur de BAD COMMANDE
    return (DDP_Errno == EBADCMD) && (data[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEBADCMD_2(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_SET;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=SUCCESS;
    acquittalFrame->dataLength=10;
    unsigned char data [] = {'s','a','l','u','t',' ','t','e','s','t'};
    acquittalFrame->data=malloc(sizeof(char) * 10);
    for(int i = 0; i<10;i++){
        acquittalFrame->data[i]=data[i];
    }
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //A_SET ne peut pas contenir de données
    //Il faut renvoyer une erreur de BAD COMMANDE
    return (DDP_Errno == EBADCMD) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowENOTDDP(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=0x55;
    acquittalFrame->dataLength=10;
    unsigned char data [] = {'s','a','l','u','t',' ','t','e','s','t'};
    acquittalFrame->data=malloc(sizeof(char) * 10);
    for(int i = 0; i<10;i++){
        acquittalFrame->data[i]=data[i];
    }
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag 0x55 n'est pas un drapeau connu du protocole DDP
    //Il faut renvoyer une erreur de NOT DDP
    return (DDP_Errno == ENOTDDP) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowENOTDDP_2(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_SET;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=0x55;
    acquittalFrame->dataLength=0;
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag 0x55 n'est pas un drapeau connu du protocole DDP
    //Il faut renvoyer une erreur de NOT DDP
    return (DDP_Errno == ENOTDDP) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEWRONGEFLAG(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=INTERNAL_ERROR;
    acquittalFrame->dataLength=10;
    unsigned char data [] = {'s','a','l','u','t',' ','t','e','s','t'};
    acquittalFrame->data=malloc(sizeof(char) * 10);
    for(int i = 0; i<10;i++){
        acquittalFrame->data[i]=data[i];
    }
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag d'erreur INTERNAL_ERROR ou NOT_FOUND n'est pas possible pour la commande A_LOOKUP contenant des données
    //Il faut donc renvoyer WRONG ERROR FLAG
    return (DDP_Errno == EWRONGEFLAG) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEWRONGNFLAG_A_SET(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_SET;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=NOT_FOUND;
    acquittalFrame->dataLength=0;
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag d'erreur NOT_FOUND n'est pas possible pour la commande A_SET
    //Il faut donc renvoyer WRONG NOT FOUND FLAG
    return (DDP_Errno == EWRONGNFLAG) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEWRONGNFLAG_A_DUMP(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_DUMP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=NOT_FOUND;
    acquittalFrame->dataLength=0;
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag d'erreur NOT_FOUND n'est pas possible pour la commande A_DUMP
    //Il faut donc renvoyer WRONG NOT FOUND FLAG
    return (DDP_Errno == EWRONGNFLAG) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEWRONGSFLAG_A_LOOKUP(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=SUCCESS;
    acquittalFrame->dataLength=0;
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Le flag d'erreur SUCCESS n'est pas possible pour la commande A_LOOKUP lorsqu'elle ne contient pas de données
    //Il faut donc renvoyer WRONG SUCCESS FLAG
    return (DDP_Errno == EWRONGSFLAG) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldThrowEWRONGLEN(){
    DDP_Errno = -1;
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=SUCCESS;
    acquittalFrame->dataLength=9;
    unsigned char data [] = {'s','a','l','u','t',' ','t','e','s','t'};
    acquittalFrame->data=malloc(sizeof(char) * 10);
    for(int i = 0; i<10;i++){
        acquittalFrame->data[i]=data[i];
    }
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //Ici la taille indiquer par la partie data length et de 9 or la longeurs du tableau de donnée ici est de 10
    //Il faut donc renvoyer une erreur de WRONG LENGTH
    return (DDP_Errno == EWRONGLEN) && (frame[0] == 0xff);
}

bool encodeAcquittalFrame_itShouldEncodeThisFrame(){
    DDP_Errno = -1;
    //10 = 0b 0000 0000 0000 1010
    //                    0x0A
    unsigned char expectedFrame[] = {A_LOOKUP, 0x2 + 1, SUCCESS,0x00 + 1,0x0A + 1,'s','a','l','u','t',' ','t','e','s','t', END_FRAME};
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=SUCCESS;
    acquittalFrame->dataLength=10;
    unsigned char data [] = {'s','a','l','u','t',' ','t','e','s','t'};
    acquittalFrame->data=malloc(sizeof(char) * 10);
    for(int i = 0; i<10;i++){
        acquittalFrame->data[i]=data[i];
    }
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //La frame et correctement formé et devrais être egale à expectedFrame
    return memcmp(expectedFrame,frame,15)==0;
}

bool encodeAcquittalFrame_itShouldEncodeThisFrame_2(){
    DDP_Errno = -1;
    unsigned char expectedFrame[] = {A_LOOKUP, 0x2 + 1, INTERNAL_ERROR, END_FRAME};
    PAcquittalFrame acquittalFrame = (PAcquittalFrame) malloc(sizeof(PAcquittalFrame));
    acquittalFrame->cmd=A_LOOKUP;
    acquittalFrame->nodeID=2;
    acquittalFrame->errorFlag=INTERNAL_ERROR;
    acquittalFrame->dataLength=0;
    unsigned char * frame = encodeAcquittalFrame(acquittalFrame);
    free(acquittalFrame);
    //La frame et correctement formé et devrais être egale à expectedFrame
    return memcmp(expectedFrame,frame,4)==0;
}

int main() {
    char *et = "evaluateType()";
    printTitle(et);
    passTest(et, "It should throw an ENOTDDP error", evaluateType_itShouldThrowsENOTDDPError());
    passTest(et, "It should return ASK type", evaluateType_itShouldBeAskFrame());
    passTest(et, "It should return ACQUITTAL type", evaluateType_itShouldBeAcquittalFrame());

    char *daf = "decodeAskFrame()";
    printTitle(daf);
    passTest(daf, "It should throw an ENOTDDP error", decodeAskFrame_itShouldThrowsENOTDDPError());
    passTest(daf, "It should throw an ENOTASK error", decodeAskFrame_itShouldThrowENOTASK());
    passTest(daf, "It should throw an EBADCMD error", decodeAskFrame_itShouldThrowEBADCMD());
    passTest(daf, "It should throw an ENOENDFLAG error", decodeAskFrame_itShouldThrowENOENDFLAG());
    passTest(daf, "It should throw another EBADCMD error", decodeAskFrame_itShouldThrowEBADCMD_2());
    passTest(daf, "It should throw another ENOENDFLAG error", decodeAskFrame_itShouldThrowENOENDFLAG_2());
    passTest(daf, "It should correctly decode this frame", decodeAskFrame_itShouldCorrectlyDecodeThisFrame());
    passTest(daf, "It should correctly decode this frame", decodeAskFrame_itShouldCorrectlyDecodeThisFrame_2());

    char *daqf = "decodeAcquittalFrame()";
    printTitle(daqf);
    passTest(daqf, "It should throw an ENOTDDP error", decodeAcquittalFrame_itShouldThrowsENOTDDPError());
    passTest(daqf, "It should throw another ENOTDDP error", decodeAcquittalFrame_itShouldThrowsENOTDDPError_2());
    passTest(daqf, "It should throw another ENOTDDP error", decodeAcquittalFrame_itShouldThrowsENOTDDPError_3());
    passTest(daqf, "It should throw ENOTDDP error cause of unknown flag",decodeAcquittalFrame_itShouldThrowsENOTDDPError_4());
    passTest(daqf, "It should throw an ENOTASK error", decodeAcquittalFrame_itShouldThrowENOTAQIT());
    passTest(daqf, "It should throw an EWRONGNFLAG with A_SET cmd",decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ASET());
    passTest(daqf, "It should throw an EWRONGNFLAG with A_DUMP cmd",decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ADUMP());
    passTest(daqf, "It should throw an EWRONGSFLAG with A_LOOKUP cmd",decodeAcquittalFrame_itShouldThrowEWRONGSFLAG_ALOOKUP());
    passTest(daqf, "It should throw an ENOENDFLAG error", decodeAcquittalFrame_itShouldThrowENOENDFLAG_2());
    passTest(daqf, "It should correctly decode this frame", decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame());
    passTest(daqf, "It should throw an EBADCMD error", decodeAcquittalFrame_itShouldThrowEBADCMD());
    passTest(daqf, "It should throw ENOTDDP error cause of unknown flag",decodeAcquittalFrame_itShouldThrowENOTDDPError_4());
    passTest(daqf, "It should throw EWRONGEFLAG error cause of unknown flag",decodeAcquittalFrame_itShouldThrowEWRONGEFLAG());
    passTest(daqf, "It should throw an ENOENDFLAG error", decodeAcquittalFrame_itShouldThrowENOENDFLAG_2());
    passTest(daqf, "It should throw an EWRONGLEN error", decodeAcquittalFrame_itShouldThrowEWRONGLEN());
    passTest(daqf, "It should correctly decode this frame", decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame_2());

    char *eaf ="encodeAskFrame()";
    printTitle(eaf);
    passTest(eaf, "It should throw an EBADCMD error", encodeAskFrame_itShouldThrowEBADCMD());
    passTest(eaf, "It should throw an ENOTDDP error", encodeAskFrame_itShouldThrowENOTDDP());
    passTest(eaf, "It should correctly encode this frame", encodeAskFrame_itShouldEncodeThisFrame());
    passTest(eaf, "It should correctly encode this frame", encodeAskFrame_itShouldEncodeThisFrame_2());

    char *eaqf ="encodeAcquittalFrame()";
    printTitle(eaqf);
    passTest(eaqf,"It should throw an EBADCMD error",encodeAcquittalFrame_itShouldThrowEBADCMD());
    passTest(eaqf,"It should throw another EBADCMD error",encodeAcquittalFrame_itShouldThrowEBADCMD_2());
    passTest(eaqf,"It should throw ENOTDDP error",encodeAcquittalFrame_itShouldThrowENOTDDP());
    passTest(eaqf,"It should throw another ENOTDDP error",encodeAcquittalFrame_itShouldThrowENOTDDP_2());
    passTest(eaqf,"It should throw another EWRONGEFLAG error",encodeAcquittalFrame_itShouldThrowEWRONGEFLAG());
    passTest(eaqf,"It should throw another EWRONGNFLAG error on A_SET",encodeAcquittalFrame_itShouldThrowEWRONGNFLAG_A_SET());
    passTest(eaqf,"It should throw another EWRONGNFLAG error on A_DUMP",encodeAcquittalFrame_itShouldThrowEWRONGNFLAG_A_DUMP());
    passTest(eaqf,"It should throw EWRONGSFLAG error on A_LOOKUP",encodeAcquittalFrame_itShouldThrowEWRONGSFLAG_A_LOOKUP());
    passTest(eaqf,"It should throw EWRONGLEN error",encodeAcquittalFrame_itShouldThrowEWRONGLEN());
    passTest(eaqf,"It should correctly encode this frame",encodeAcquittalFrame_itShouldEncodeThisFrame());
    passTest(eaqf,"It should correctly encode this frame",encodeAcquittalFrame_itShouldEncodeThisFrame_2());

    resume();
}



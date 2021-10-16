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
    printf(COLOR_YELLOW"\n%d teste réussie sur %d (%d/%d)"COLOR_RESET, testPassed, totalTest, testPassed, totalTest);
}

/*****************************
 *  test de evaluateType()  *
 ****************************/
bool evaluateType_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    char val = evaluateType(0xff);
    return (DDP_Errno == ENOTDDP) && (val == -1);
}

bool evaluateType_itShouldBeAskFrame() {
    char frame = evaluateType(C_SET);
    return frame == ASK;
}

bool evaluateType_itShouldBeAcquittalFrame() {
    char frame = evaluateType(A_SET);
    return frame == ACQUITTAL;
}

/******************************
 *  test de decodeAskFrame()  *
 ******************************/
bool decodeAskFrame_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 'n', 'o', 't', ' ', 'a', ' ', 'f', 'r', 'a', 'm'};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOTASK() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_SET, 2, SUCCESS, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == ENOTASK) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowEBADCMD() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOENDFLAG() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_EXIT, 2};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowEBADCMD_2() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_EXIT, 0xA, 0xA, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldThrowENOENDFLAG_2() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 0xA, 0xA, 0xFF};
    PAskFrame res = decodeAskFrame(frame);
    return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAskFrame_itShouldCorrectlyDecodeThisFrame() {
    DDP_Errno = -1;
    /**
     * On veut envoyer la valeur 125 soit 0b0000 0000 0111 1101
     * on envoie donc 0 0x7D
     */
    unsigned char frame[] = {C_SET, 0 + 1, 0x7D + 1, END_FRAME};
    PAskFrame res = decodeAskFrame(frame);
    return (res->cmd == C_SET) && (res->val == 125);
}

/************************************
 *  test de decodeAcquittalFrame()  *
 ************************************/
bool decodeAcquittalFrame_itShouldThrowsENOTDDPError() {
    DDP_Errno = -1;
    //permet de tester pour une taille non détérminer supérieur à 4
    unsigned char frame[] = {A_LOOKUP, 'n', 'o', 't', ' ', 'a', ' ', 'f', 'r', 'a', 'm'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_2() {
    DDP_Errno = -1;
    //Permet de tester pour une taille de 4 pille
    unsigned char frame[] = {A_SET, 'n', 'o', 't'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_3() {
    DDP_Errno = -1;
    //Permet de tester pour une taille inférieur à 4
    unsigned char frame[] = {A_SET, 'n', 'o'};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowsENOTDDPError_4() {
    DDP_Errno = -1;
    //Une frame avec un drapeau inconue n'est pas du ddp protocole
    unsigned char frame[] = {A_LOOKUP, 1, 0x55, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOTAQIT() {
    DDP_Errno = -1;
    unsigned char frame[] = {C_SET, 250, SUCCESS, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTAQIT) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ASET() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_SET, 1, NOT_FOUND, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == EWRONGNFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGNFLAG_ADUMP() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_DUMP, 1, NOT_FOUND, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == EWRONGNFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGSFLAG_ALOOKUP() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 1, SUCCESS, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == EWRONGSFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOENDFLAG() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 1, INTERNAL_ERROR, 0x55};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame() {
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, INTERNAL_ERROR, END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
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
    return (DDP_Errno == EBADCMD) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOTDDPError_4(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, 0x55, 0x0+1,0x2+1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == ENOTDDP) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGEFLAG(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, NOT_FOUND, 0x0+1,0x2+1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == EWRONGEFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowENOENDFLAG_2(){
        DDP_Errno = -1;
        unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0+1,0x2+1,'h','i',0xEE};
        PAcquittalFrame res = decodeAcquittalFrame(frame);
        return (DDP_Errno == ENOENDFLAG) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldThrowEWRONGLEN(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0 + 1,0x3 + 1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
    return (DDP_Errno == EWRONGLEN) && (res->cmd == 0xff);
}

bool decodeAcquittalFrame_itShouldCorrectlyDecodeThisFrame_2(){
    DDP_Errno = -1;
    unsigned char frame[] = {A_LOOKUP, 0 + 1, SUCCESS, 0x0 + 1,0x2 + 1,'h','i',END_FRAME};
    PAcquittalFrame res = decodeAcquittalFrame(frame);
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
    return (DDP_Errno == EBADCMD) && (data[0] == 0xff);
}

bool encodeAskFrame_itShouldThrowENOTDDP(){
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=0x51;
    askFrame->val=2;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    return (DDP_Errno == ENOTDDP) && (data[0] == 0xff);
}

bool encodeAskFrame_itShouldEncodeThisFrame(){
    //25 = 0b 0000 0000 0001 1001
    //                     0x19
    unsigned char expectedFrame[] = {C_LOOKUP, 0x0 + 1, 0x19 + 1, END_FRAME};
    DDP_Errno = -1;
    PAskFrame askFrame = (PAskFrame) malloc(sizeof(AskFrame));
    askFrame->cmd=C_LOOKUP;
    askFrame->val=25;
    unsigned char * data = encodeAskFrame(askFrame);
    free(askFrame);
    return memcmp(expectedFrame,data,4)==0;
}

//
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
    passTest(daqf, "It should throw an ENOENDFLAG error", decodeAcquittalFrame_itShouldThrowENOENDFLAG());
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
    passTest(eaf, "It should throw an ENOTDDP error", encodeAskFrame_itShouldEncodeThisFrame());

    resume();
}



//
// Created by louis on 12/10/2021.
//

#include "DDP.h"
#include <stdio.h>

int DDP_Errno=-1;
const char * DDP_errList[]= {
        "Not a DDP Frame",
        "CMD not match with frame",
        "Node isn't valid",
        "Error flag placed but data was found",
        "Success flag placed but no data was found",
        "The length don't match with data",
        "No end flag in stream",
        "Not a ASK FRAME",
        "Not a ACQUITTEMENT FRAME",
};;

char evaluateFrame(char *frame){
    DDP_Errno = EBADCMD;
    return -1;
}

PAskFrame decodeAskFrame(char *frame){
    return NULL;
}

PAcquittalFrame decodeAcquittalFrame(char *frame) {
    return NULL;
}

char *encodeAskFrame(PAskFrame askFrame){
    return (char *) (char) -1;
}

char *encodeAcquittalFrame(PAcquittalFrame acquittalFrame){
    return (char *) (char) -1;
}

void DDP_perror(char * data){
    if(DDP_Errno>=0 && DDP_Errno<=8){
        fprintf(stderr, "%s : %s\n", data, DDP_errList[DDP_Errno]);
    } else{
        fprintf(stderr,"%s : Success\n",data);
    }
}
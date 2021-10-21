#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "controller.h"



int main(){
    unsigned char stream[] = {0xd2,0x12,0x20,0x04,0xd2,0x13,0x20,0x04,0xd2,0x14,0x20,0x04,0xd2,0x15,0x20,0x04,0xd2,0x16,0x20,0x04,0xd2,0x17,0x20,0x04,0xd2,0x18,0x20,0x04,0xd2,0x19,0x20,0x04,0xd2,0x1a,0x20,0x04,0xd2,0x1b,0x20,0x04,0xd2};
    printf("aquittal : %d\n",countAcquittal);
    PAcquittalFrame frame = deserialization(stream,44,512);
    printf("aquittal %d should be 7 cmd : %d\n",countAcquittal,frame->cmd);
    unsigned char str[] = {0xd2,0x91,0x20,0x4};
    PAcquittalFrame frame1 = decodeAcquittalFrame(str,4);
    printf("cmd : %x",frame1->cmd);
    DDP_perror("Erreur ?");

    return 0;
}

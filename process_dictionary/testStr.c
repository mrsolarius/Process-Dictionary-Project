#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_hex(const unsigned char *s)
{
    while(*s)
        printf("%02x", (unsigned int) *s++);
    printf("\n");
}

int main(){
    unsigned char stream[] = {0xd2,0x2,0x20,0x4,0xd2,0x3,0x20,0x4,0xd2,0x4,0x20,0x4,0xd2,0x5,0x20,0x4};
    unsigned char * des = malloc(sizeof(unsigned char)*1);
    strncpy(des, stream,1);
    print_hex(des);
    strncpy(des, stream,2);
    print_hex(des);
    strncpy( des, stream,3);
    print_hex(des);
    strncpy( des, stream,4);
    print_hex(des);
    free(des);
    printf("On déce 4 là\n");
    strncpy( des, stream+4,1);
    print_hex(des);
    strncpy( des, stream+4,2);
    print_hex(des);
    strncpy( des, stream+4,3);
    print_hex(des);
    strncpy( des, stream+4,4);
    print_hex(des);
    return 0;
}

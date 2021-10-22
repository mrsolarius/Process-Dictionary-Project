#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "controller.h"

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

void print_hex(const unsigned char *s)
{
    while(*s)
        printf("%02x-", (unsigned int) *s++);
    printf("\n");
}

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

bool itShouldCount12Frame(){
    countAcquittal = 0 ;
    unsigned char stream[] = {0xd2,0x12,0x20,0x04,0xd2,0x13,0x20,0x04,0xd2,0x14,0x20,0x04,0xd2,0x15,0x20,0x04,0xd2,0x16,0x20,0x04,0xd2,0x17,0x20,0x04,0xd2,0x18,0x20,0x04,0xd2,0x19,0x20,0x04,0xd2,0x1a,0x20,0x04,0xd2,0x1b,0x20,0x04,0xd2,0x91,0x20,0x4,0xd2,0x92,0x20,0x4,0xd2};
    PAcquittalFrame frame = deserialization(stream,52,254);
    return countAcquittal == 12;
}

int main(){
    char *dz = "deserialization()";
    printTitle(dz);
    passTest(dz,"It should place countAcquittal at 12",itShouldCount12Frame());
    resume();
    return 0;
}

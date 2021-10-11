#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Argument insufisant\n");
        exit(-1);
    }
    int nbFis;
    nbFis = atoi(argv[1]);
    if (nbFis == -1) {
        fprintf(stderr, "Ce n'est pas un nb\n");
        exit(-1);
    }
    for (int i = 0; i < nbFis; i++) // Generer n fis
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        if (fork() == 0) {
            printf("[fis] pid %d [père] pid %d\n", getpid(), getppid());
            exit(0);
        }
    }
    //On attend là mort de tous nos processus
    while (wait(NULL) != -1);
    return 0;
}

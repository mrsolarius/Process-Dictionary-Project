#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int execFis(int appID, int pipeCtr[2], int pipeRead[2], int pipeWrite[2]){
    int buffNode=0;
    long int res;

    //Avant de commencer notre traitement on ferme les lecture/ecriture qui ne nous serve pas sur nos différent pipes
    //On ferme en lecture le pipe du controller
    close(pipeCtr[0]);
    //On ferme en ecriture le pipe de lecture
    close(pipeRead[1]);
    //On ferme en lecture le pipe d'écriture
    close(pipeWrite[0]);

    //On Lis le tube du précédent du node
    res = read(pipeRead[0], &buffNode, sizeof(int));
    if (res==-1) perror("Probléme de lecture");
    close(pipeRead[0]);

    //On Affiche le contenu du node précédent
    printf("id:%d buffNode:%d\n", appID, buffNode);
    //On l'incrémente de 1 pour s'assurer que l'opération et executer sur tous les node
    buffNode += 1;

    //on envois se nouveau contenue au node suivant
    res = write(pipeWrite[1], &buffNode, sizeof(int));
    if(res==-1)perror("Probléme d'écriture node");
    close(pipeWrite[1]);


    //On écrit l'id du processus dans le tub en direction du controller
    res = write(pipeCtr[1],&appID,sizeof(int));
    if(res==-1)perror("Probléme d'écriture ctrl");
    close(pipeCtr[1]);

    //on quite le process
    exit(0);
}

int main(int argc, char *argv[]) {
    int nbFis;

    if (argc != 2) {
        fprintf(stderr, "Argument insufisant\n");
        exit(-1);
    }
    nbFis = atoi(argv[1]);
    if (nbFis == -1) {
        fprintf(stderr, "Ce n'est pas un nb\n");
        exit(-1);
    }

    if(nbFis<2){
        fprintf(stderr, "Au moins 2 processus son requis\n");
        exit(-1);
    }

    //Inisialisation d'un tableau de tableau pour générer tous les pipe
    int* pipeArr[nbFis];
    for (int i = 0; i < nbFis; i++) {
        pipeArr[i] = (int *) malloc(2 * sizeof(int));
        pipe(pipeArr[i]);
    }

    //Injection de la premier valeur au node 0
    int val = 500;
    write(pipeArr[nbFis-1][1],&val,sizeof(int));

    int pipeCtrl[2];
    //On ouvre le pipe en direction du controller
    pipe(pipeCtrl);
    for (int i = 0; i < nbFis; i++) // Generer n fis
    {
        //on ne s'occuper ici que des fis pour afficher leur pid et celui de leur père
        if (fork() == 0) {
            if(i==0){
                execFis(i,pipeCtrl,pipeArr[nbFis-1],pipeArr[i]);
            }else{
                execFis(i,pipeCtrl,pipeArr[i-1],pipeArr[i]);
            }
        }
    }
    int buffCtrl;
    //On lis les message envoyer par les fis
    while (read(pipeCtrl[0], &buffCtrl, sizeof(int)) > 0 && buffCtrl != nbFis - 1){
        printf("Signal receive : %d\n", buffCtrl);
    }
    printf("Signal receive : %d\n", buffCtrl);
    close(pipeCtrl[1]);

    //On libère du heap les tableaux et on attend les fis
    for (int i = 0; i < nbFis; i++) {
        free(pipeArr[i]);
        wait(NULL);
    }

    return 0;
}

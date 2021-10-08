#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    for(int i=0;i<5;i++) // Generer 5 fis
    {
        if(fork() == 0)
        {
            printf("[fis] pid %d [père] pid %d\n",getpid(),getppid());
            exit(0);
        }
    }
    for(int i=0;i<5;i++) // attendre la génération des fis
    wait(NULL);
    return 0;
}

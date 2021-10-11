#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    short val = -1;
    int key;
    char *value = NULL;
    Table_entry *tabletest = NULL;
    do {
        printf("\nSaisir commande (0 = exit, 1 = set, 2 = lookup, 3 = dump):");
        scanf("%hd", &val);
        switch (val) {
            case 0:
                printf("bye bye !");
                break;
            case 1:
                printf("Saisir la cle (decimal number): ");
                scanf("%d", &key);
                printf("Saisir la valeur (chaine de caracteres): ");
                scanf(" %ms", &value);
                if (value == NULL)
                    fprintf(stderr, "That string was too long!\n");
                else {
                    store(&tabletest, key, value);
                    free(value);
                }
                break;
            case 2:
                printf("Saisir la cle (decimal number): ");
                scanf("%d", &key);
                value = lookup(&tabletest, key);
                if (value == NULL) {
                    printf("Pas de valeur trouvee");
                }
            case 3:
                display(tabletest);
                break;
            default:
                printf("\nVotre commande n'est pas reconue veuillez recomancer");
                break;
        }

    } while (val != 0);
    free(tabletest);
    return 0;
}
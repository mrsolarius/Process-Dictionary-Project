//
// Created by louis on 12/10/2021.
//

#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stocke dans table la valeur v sous la cle k
void store(PTable_entry *table, int k, char v[]) {
    PTable_entry te = (PTable_entry) malloc(sizeof(Table_entry));
    te->key = k;
    te->val = (char*) malloc(strlen(v) + 1);
    strcpy(te->val, v);
    te->next = *table;
    *table = te;
}
// Retourne la valeur se trouvant dans table sous la cle k, ou NULL si la
// cle n'existe pas
char* lookup(PTable_entry table, int k) {
    PTable_entry pe = table;
    while (pe != NULL && pe->key != k) {
        pe = pe->next;
    }
    if (pe != NULL)
        return pe->val;
    else
        return NULL;
}
// Affiche le contenu de la table
void display(PTable_entry table) {
    PTable_entry pe = table;
    while (pe != NULL) {
        printf("%d : %s\n", pe->key, pe->val);
        pe = pe->next;
    }
}
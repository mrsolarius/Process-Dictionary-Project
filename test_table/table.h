#ifndef PROCESS_DICTIONARY_PROJECT_TABLE_H
#define PROCESS_DICTIONARY_PROJECT_TABLE_H

typedef struct _Table_entry {
    int key;                     // cle
    char* val;                   // valeur
    struct _Table_entry *next;   // suivant
} Table_entry;

typedef Table_entry *PTable_entry;
char* lookup(PTable_entry table, int k);
void store(PTable_entry *table, int k, char v[]);
void display(PTable_entry table);

#endif //PROCESS_DICTIONARY_PROJECT_TABLE_H

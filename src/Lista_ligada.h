#ifndef Lista_ligada_H
#define Lista_ligada_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"

typedef struct ligada {
    Informacao info;
    struct ligada *prox;
} *LIGADA;

LIGADA initLista();
void insereElem(Informacao info, LIGADA* lista);
int removeElem(pid_t pid, LIGADA* lista);
Informacao* Lookup_struct(pid_t pid, LIGADA lista);
void pritnLista (LIGADA lista);
int tamLista(LIGADA *lista);

#endif
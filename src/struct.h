#ifndef STRUCT_H
#define STRUCT_H
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h> 
#include <string.h>
#include <stdlib.h>

typedef struct Informacao{
    pid_t pid;
    char nome[50];
    long tempo;
} Informacao;

Informacao new_Info (pid_t pid, char* nome , long tempo);

#endif
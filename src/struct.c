#include "struct.h"
#define FREE(algo) if (algo) free(algo)

Informacao new_Info (pid_t pid, char* nome , long tempo){

    Informacao info;
    info.pid = pid;
    strcpy(info.nome, nome);
    info.tempo = tempo;

    return info;
}


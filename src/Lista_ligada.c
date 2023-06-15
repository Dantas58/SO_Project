#include "Lista_ligada.h"

LIGADA initLista(){
    LIGADA lista = NULL;
    return lista;
}

// Esta função insere o elemento à cabeça da lista ligada
void insereElem(Informacao info, LIGADA* lista){
    LIGADA l = malloc(sizeof(struct ligada));
    if (l == NULL) {
        perror("malloc failed");
    }
    l->info = info;
    l->prox = (*lista);
    (*lista) = l;
}

// Devolve o número de elementos na lista ligada
int tamLista(LIGADA *lista){
    int tam = 0;

    while((*lista) != NULL){
        tam++;
        lista = &(*lista)->prox;
    }

    return tam;
}

// Remove da lista ligada um programa que já terminou a sua execução
int removeElem(pid_t pid, LIGADA* lista){
    Informacao aux;
    int removido = 0;

    while((*lista) != NULL && removido == 0){
        if((*lista)->info.pid == pid){
            LIGADA temp = *lista;
            aux = (*lista)->info;
            (*lista) = (*lista)->prox;
            free(temp); // Liberta a memória do elemento removido
            removido = 1;
        }else{
            lista = &(*lista)->prox;
        }
    }
    return removido;   
}

Informacao* Lookup_struct(pid_t pid, LIGADA lista) {
    while (lista != NULL) {
        if (lista->info.pid == pid) {
            return &(lista->info);
        }
        lista = lista->prox;
    }
    return NULL;
}

void pritnLista (LIGADA lista){
    while(lista != NULL){
        printf("PID: %d\n", lista->info.pid);
        printf("Nome: %s\n", lista->info.nome);
        printf("Tempo: %ld\n", lista->info.tempo);
        lista = lista->prox;
    }

}
#include "monitor_func.h"

#define FREE(algo) if (algo) free(algo)


void executeU (int fd_fifo_Pid, LIGADA *lista){

    Informacao info;
    
    read(fd_fifo_Pid, &info, sizeof(Informacao));
    printf("nome: %s\n", info.nome);
    insereElem(info, lista);

}

void executeU_final(int fd_fifo_Pid, LIGADA *lista, char *path){
    
    Informacao info;

    //pritnLista(*lista);
    read(fd_fifo_Pid, &info, sizeof(Informacao));


    Informacao *i = Lookup_struct(info.pid, *lista);
    if(i == NULL) {
        perror("error looking up struct");
    }
    
    printf("Runing PID %d\n", info.pid);
    
    
    char *pid_str = malloc(sizeof(char)*20);
    if (pid_str == NULL) {
        perror("malloc failed");
    }
    
    sprintf(pid_str, "/%d", info.pid);
    char *path_pid = malloc(sizeof(char)*200); 
    path_pid[0] = '\0'; // inicializar a string
    strcat(path_pid, "src/");
    strcat(path_pid, path);
    strcat(path_pid, pid_str);
    FREE(pid_str);
    
    int fd = open(path_pid, O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if(fd == -1) {
        perror("error opening fifo f");
    }

    long diff = info.tempo - i->tempo;
    i->tempo = diff;
    printf("Ended in %ld ms\n", diff);
    write(fd, i, sizeof(Informacao));
    int n = removeElem(info.pid, lista);
    if(n == 0) {
        perror("error removing element");
    }
    pritnLista(*lista);
    
    FREE(path_pid);
    close(fd);  

} 

void status(int fd_fifo_Pid, LIGADA *lista){
    
    struct timeval agora;
    gettimeofday(&agora, NULL);
    long agora_ms = agora.tv_sec * 1000L + agora.tv_usec / 1000L;

    pritnLista(*lista); 
    LIGADA p = *lista;
    int tam = tamLista(lista);
    
    int bytes = write(fd_fifo_Pid, &tam, sizeof(int));
    
    if (bytes == -1) {
        perror("error writing to fifo");
    }   

    while (p != NULL) { 
        Informacao* i = &(p->info);
        i->tempo = agora_ms - i->tempo;
        write(fd_fifo_Pid, i, sizeof(Informacao));
        p = p->prox; 
    }
}


void status_time (char* pids, int fd_fifo_pid, int l){
    int i = 0;
    long tempo = 0;
    char *copy = strdup(pids);
    char *token, *comandos[l];

    while ((token = strsep(&copy, " ")) != NULL && i < l) {
        comandos[i] = strdup(token);
        i++;
    }
    
    comandos[i] = NULL;

    for (i = 0; comandos[i] != NULL; i++) {
        //printf("comando: %s\n", comandos[i]);
        char path[64] = "src/PIDS_folder/";
        strcat(path, comandos[i]);
        int fd_ficheiro = open(path, O_RDONLY);
        Informacao info;
        read(fd_ficheiro, &info, sizeof(Informacao));
        //printf("tempo: %ld\n", info.tempo);
        tempo += info.tempo;
    }
    printf("tempo total: %ld\n", tempo);
    write(fd_fifo_pid, &tempo, sizeof(long));
    

}

void status_comand (char* prog, char* pids, int l_pids, int fd_fifo_pid){
    int r = 0;
    int i = 0;
    long tempo = 0;
    char *copy = strdup(pids);
    char *token, *comandos[l_pids];

    while ((token = strsep(&copy, " ")) != NULL && i < l_pids) {
        comandos[i] = strdup(token);
        i++;
    }
    
    comandos[i] = NULL;

    for(i = 0; comandos[i]!=NULL; i++){
        char path[64] = "src/PIDS_folder/";
        strcat(path, comandos[i]);
        int fd_ficheiro = open(path, O_RDONLY);
        Informacao info;
        read(fd_ficheiro, &info, sizeof(Informacao));
        if(strchr(info.nome, '|') !=NULL){
            char *token, *comandos2[100];
            int j = 0;
            char *copy2 = strdup(info.nome);
            while ((token = strsep(&copy2, "|")) != NULL && j < 100) {
                while (isspace(*token)) token++;
                char *end = token + strlen(token) - 1;
                while (end > token && isspace(*end)) end--;
                *(end + 1) = 0;
                // Store the command in the array
                comandos2[j] = strdup(token);
                j++;
            }
            comandos2[j] = NULL;

            printf("comandos: %s %s\n", comandos2[0], comandos2[1]);

            for(int x = 0; comandos2[x]!=NULL; x++){
                if(strcmp(comandos2[x], prog)== 0){
                    r++;
                }
            }
        }
        else{
            if(strcmp(info.nome, prog) == 0){
                r++;
            }
        }

    }

    printf("numero de vezes que o programa %s foi executado: %d\n", prog, r);
    write(fd_fifo_pid, &r, sizeof(int));

}


int existe(char* nome, char** nomes, int l){
    int i;
    for(i = 0; i < l; i++){
        if(strcmp(nome, nomes[i]) == 0){
            return 1;
        }
    }
    return 0;
}


void status_uniq (char* pids, int l_pids, int fd_fifo_pid){
    int r = 0;
    int i = 0;
    long tempo = 0;
    char *copy = strdup(pids);
    char *token, *comandos[l_pids];

    while ((token = strsep(&copy, " ")) != NULL && i < l_pids) {
        comandos[i] = strdup(token);
        i++;
    }
    
    comandos[i] = NULL;

    char *nomes[l_pids];
    int k = 0;
    
    for(i=0; comandos[i]!=NULL; i++){
        char path[64] = "src/PIDS_folder/";
        strcat(path, comandos[i]);
        int fd_ficheiro = open(path, O_RDONLY);
        Informacao info;
        read(fd_ficheiro, &info, sizeof(Informacao));
        if(strchr(info.nome, '|') !=NULL){
            char *token, *comandos2[100];
            int j = 0;
            char *copy2 = strdup(info.nome);
            while ((token = strsep(&copy2, "|")) != NULL && j < 100) {
                while (isspace(*token)) token++;
                char *end = token + strlen(token) - 1;
                while (end > token && isspace(*end)) end--;
                *(end + 1) = 0;
                // Store the command in the array
                comandos2[j] = strdup(token);
                j++;
            }
            comandos2[j] = NULL;
            
            for(int x = 0; comandos2[x]!=NULL; x++){
                if(!existe(comandos2[x], nomes, k)){
                    nomes[k] = strdup(comandos2[x]);
                    //printf("nome: %s\n", nomes[k]);
                    k++;
                }
            }   
        }
        else{
            if(!existe(info.nome, nomes, k)){
            nomes[k] = strdup(info.nome);
            //printf("nome: %s\n", nomes[k]);
            k++;
            }
        }
           
    }

    write(fd_fifo_pid, &k, sizeof(int));
    nomes[k] = NULL;

    for(k=0; nomes[k]!=NULL;k++){
        //printf("nome: %s\n", nomes[k]);
        int l = strlen(nomes[k]);
        //printf("l: %d\n", l);
        write(fd_fifo_pid, &l, sizeof(int));
        write(fd_fifo_pid, nomes[k], (l+1)*sizeof(char));
    }

}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "struct.h"


int execute (char *c, int l, int fd) {
    struct timeval antes;
    struct timeval depois;
    int tempo;
    int status;
    int i = 0;
    char *copy = strdup(c);
    char *func = strsep(&copy, " ");
    char *comando[l];

    // execução do comando -u
    if (strcmp(func, "-u") == 0){
        char *token;

        while ((token = strsep(&copy, " ")) != NULL && i < l) {
            
            if (token[0] == '"' && token[strlen(token)-1] == '"') {
                token[strlen(token)-1] = '\0'; 
                token++; 
            }
    
            comando[i] = strdup(token);
            i++;
        }
    
        comando[i] = NULL;
        pid_t pid_pai = getpid();
        char pid_str_fifo[20];
        sprintf(pid_str_fifo, "tmp/%ld", pid_pai);
                
        
        mkfifo(pid_str_fifo, 0666);
        
        int fd_w;
        write(fd, &pid_pai, sizeof(pid_t));
        fd_w = open(pid_str_fifo, O_WRONLY);
        int mensagem = 10;
        write(fd_w, &mensagem, sizeof(int)); // mensagem para executar executeU


        int file_d[2];
        
        if(pipe(file_d)==-1){
            perror("erro a abrir pipe");
        }

        pid_t pid = fork();

        if (pid == 0) {
            close(file_d[1]);
            char ler[20];
            read(file_d[0],ler,17*sizeof(char));
            int c;
            c = execvp(comando[0], comando);
            _exit(c);
        }
        
        
        close(file_d[0]);

        char *comando_str = malloc(sizeof(char)*200);
        comando_str[0] = '\0'; // inicializar a string 
        for (int i = 0; comando[i] != NULL; i++) {
            strcat(comando_str, comando[i]);
            if(comando[i+1] != NULL){
                strcat(comando_str, " ");
            }
        }
        
        tempo = gettimeofday (&antes, NULL);

        long antes_ms = antes.tv_sec * 1000L + antes.tv_usec / 1000L;
        
        char *nome = strdup(comando[0]);
        printf("Running PID %d\n", pid);

        Informacao info = new_Info(pid, comando_str, antes_ms); 

        free(comando_str);
        
        // escrita do pid do processo a ser executado no fifo do programa monitor
        write(fd_w, &info, sizeof(Informacao));
        close(fd_w);   

        write(file_d[1],"desbloqueia filho", 17*sizeof(char)); // bloquear o filho até ele ler o que foi escrito no pipe

        close(file_d[1]);
        
        if(tempo!=0){
                perror("Fim getimeofday falhou");
                return -1;
        }

        pid_t pid_processo_term = wait(&status);
        if(pid_processo_term == -1){
            perror("erro no wait");
        }
        
        write(fd, &pid_pai, sizeof(pid_t));
        fd_w = open(pid_str_fifo, O_WRONLY);
        mensagem = 11;
        write(fd_w, &mensagem, sizeof(int)); // mandar mensagem para executar executeU_final
        
        
        if(WIFEXITED(status)){
            tempo = gettimeofday(&depois, NULL);
            long depois_ms = depois.tv_sec * 1000L + depois.tv_usec / 1000L;
            
            if(tempo!=0){
                perror("Fim getimeofday falhou");
                return -1;
            }

            long diff = depois_ms - antes_ms;
            printf("Ended in %ld ms\n", diff);
            
            Informacao info2 = new_Info(pid, " ", depois_ms);

            write(fd_w, &info2, sizeof(Informacao));

        }

        for (i = 0; comando[i] != NULL; i++) {
            free(comando[i]);
        }
        
        // close the fifo
        close(fd);
        close(fd_w);
        unlink(pid_str_fifo);

    }
    // execução do comando -p
    else if(strcmp(func, "-p")==0){
        int contador = 0;
        char *copy2 = strdup(copy);
        for (int i = 0; copy[i] != '\0'; i++) {
          if (copy[i] == '|') {
            contador++;
          }
        }

        int number_processes = contador+1;
        int fd_p[number_processes-1][2];
        char *cmd[number_processes][100];
        char *token;
        int i,j = 0;

        for (i = 0; i < number_processes; i++) {
            for (j = 0; j < 100; j++) {
                cmd[i][j] = malloc(sizeof(char) * 100);
            }
        }

        for(j= 0; j< number_processes; j++){
            i=0;
            while ( (token = strsep(&copy, " ")) != NULL ) {
                //printf("token: %s\n",token);
                if(strcmp(token, "|")==0)break;
                //printf("token2: %s\n",token);
                cmd[j][i] = token;
                i++;
            }
            cmd[j][i] = NULL;
        }

        printf("number_processes: %d\n", number_processes);
        pid_t pid = getpid();
        char pid_str_fifo[20];
        sprintf(pid_str_fifo, "tmp/%ld", pid);
                
        
        mkfifo(pid_str_fifo, 0666);
        
        write(fd, &pid, sizeof(pid_t));
        
        int fd_w = open(pid_str_fifo, O_WRONLY);
        if(fd_w<0){
            perror("erro a abrir fifo");
            return -1;
        }
        int mensagem = 10;
        write(fd_w, &mensagem, sizeof(int));
        

        int tempo = gettimeofday (&antes, NULL);
        long antes_ms = antes.tv_sec * 1000L + antes.tv_usec / 1000L;
        Informacao info = new_Info(pid, copy2, antes_ms);

        write(fd_w, &info, sizeof(Informacao));
        close(fd_w);

        for (i = 0; i <number_processes; i++){
            if(i==0){

                if (pipe(fd_p[i])<0){
                    perror("erro a criar pipe anónimo");
                    return -1;
                }

                if (fork()==0){

                    close(fd_p[i][0]);

                    if (dup2(fd_p[i][1],1)<0){
                        perror("[filho] erro dup2");
                        _exit(-1);
                    }

                    close(fd_p[i][1]);

                    execvp(cmd[i][0], cmd[i]);
                    _exit(0);
                }
                else{
                    close(fd_p[i][1]);
                }
                }else if(i== number_processes -1){
                    if(fork()==0){
                        dup2(fd_p[i-1][0],0);
                        close(fd_p[i-1][0]);

                        execvp(cmd[i][0], cmd[i]);
                        _exit(0);
                    }else{
                        close(fd_p[i-1][0]);
                    }
                } 
                else{
                    if (pipe(fd_p[i])<0){
                        perror("erro a criar pipe anónimo");
                        return -1;
                    }
                    if (fork()==0){

                        close(fd_p[i][0]);

                        if (dup2(fd_p[i-1][0],0)<0){
                            perror("[filho] erro dup2");
                            _exit(-1);
                        }
                        close(fd_p[i-1][0]);

                        if (dup2(fd_p[i][1],1)<0){
                            perror("[filho] erro dup2");
                            _exit(-1);
                        }

                        close(fd_p[i][1]);

                        execvp(cmd[i][0], cmd[i]);

                        _exit(0);
                }else{
                    close(fd_p[i-1][0]);
                    close(fd_p[i][1]);
                }
            }
        }
        for(int i=0; i<number_processes; i++){
            wait(NULL);
        }

        write(fd, &pid, sizeof(pid_t));
        fd_w = open(pid_str_fifo, O_WRONLY);
        mensagem = 11;
        write(fd_w, &mensagem, sizeof(int));

        tempo = gettimeofday(&depois, NULL);
        long depois_ms = depois.tv_sec * 1000L + depois.tv_usec / 1000L;
        long diff = depois_ms - antes_ms;
        printf("Ended in %ld ms\n", diff);

        Informacao info2 = new_Info(pid, " ", depois_ms);

        write(fd_w, &info2, sizeof(Informacao));
        close(fd);
        close(fd_w);
        unlink(pid_str_fifo);
        free(copy2);
    }

    free(copy);
     

    return 0;
}


void status (int fd_w){
    
    int bytes;
    pid_t pid_pai = getpid();
    char pid_str_fifo[20];
    
    
    sprintf(pid_str_fifo, "tmp/%ld", pid_pai);
    
    mkfifo(pid_str_fifo, 0666);
    
    write(fd_w, &pid_pai, sizeof(pid_t));
    
    int fd_pid;
    fd_pid = open(pid_str_fifo, O_WRONLY);

    if(fd_pid == -1){
        printf("erro a abrir fifo do status cliente\n");
    }

    
    int mensagem = 12;
    write(fd_pid, &mensagem, sizeof(int)); // mensagem para executar status
   
    char pid_str_status[20];
    sprintf(pid_str_status, "tmp/%ld25", pid_pai);
    mkfifo(pid_str_status, 0666); 
    write(fd_pid, &pid_pai, sizeof(int));
    int fd_status = open(pid_str_status, O_RDONLY);
    close(fd_pid);
    
    Informacao info;
    int tam = 0;
    read(fd_status, &tam, sizeof(int));
    
    
    for(int i = 0; i < tam; i++){
        read(fd_status,&info,sizeof(Informacao));
        printf("%d %s %d ms\n", info.pid, info.nome, info.tempo);
    }

    close(fd_status);
    unlink(pid_str_status);
    unlink(pid_str_fifo);
}


void stats_time (int fd_w, char* comandos){
    //printf("comandos: %s\n", comandos);
    int bytes;
    pid_t pid_pai = getpid();
    char pid_str_fifo[20];
    
    //printf("pid pai %ld\n", pid_pai);
    sprintf(pid_str_fifo, "tmp/%ld", pid_pai);
    //printf("%s\n",pid_str_fifo);
    mkfifo(pid_str_fifo, 0666);   // criar fifo com o pid do pai
    
    write(fd_w, &pid_pai, sizeof(pid_t));
    
    int fd_pid;
    fd_pid = open(pid_str_fifo, O_WRONLY);

    if(fd_pid == -1){
        printf("erro a abrir fifo do status cliente\n");
    }

    int mensagem = 13;
    write(fd_pid, &mensagem, sizeof(int)); // mensagem para executar status-time
    
    char pid_str_status[20];
    sprintf(pid_str_status, "tmp/%ld25", pid_pai);
    int pid_status = atoi(pid_str_status);
    mkfifo(pid_str_status, 0666);
    write(fd_pid, &pid_pai, sizeof(int));
    int fd_status = open(pid_str_status, O_RDONLY);
    
    
    int length = strlen(comandos);
    //printf("length: %d\n", length);
    bytes = write(fd_pid, &length, sizeof(int)); // tamanho do comando
    if (bytes == -1){
        perror("write");
    }
    
    //printf("bytes: %d\n", bytes);
    write(fd_pid, comandos, (length+1)*sizeof(char)); // enviar os pids a consultar
    
    close(fd_pid);
    
    int tempo;
    read(fd_status, &tempo, sizeof(int));
    printf("Total execution time is %d ms \n", tempo);

    close(fd_status);
    unlink(pid_str_status);
    unlink(pid_str_fifo);
}

void status_comand(char* prog, char* pids, int fd_w){
    //printf("comandos: %s\n", prog);
    //printf("pids: %s\n", pids);
    int bytes;
    pid_t pid_pai = getpid();
    char pid_str_fifo[20];
    
    //printf("pid pai %ld\n", pid_pai);
    sprintf(pid_str_fifo, "tmp/%ld", pid_pai);
    //printf("%s\n",pid_str_fifo);
    mkfifo(pid_str_fifo, 0666);

    write(fd_w, &pid_pai, sizeof(pid_t));
    
    int fd_pid;
    fd_pid = open(pid_str_fifo, O_WRONLY);

    if(fd_pid == -1){
        printf("erro a abrir fifo do status cliente\n");
    }

    int mensagem = 14;
    write(fd_pid, &mensagem, sizeof(int)); // mensagem para executar status-time

    char pid_str_status[20];
    sprintf(pid_str_status, "tmp/%ld25", pid_pai);
    int pid_status = atoi(pid_str_status);
    mkfifo(pid_str_status, 0666);
    write(fd_pid, &pid_pai, sizeof(int));
    int fd_status = open(pid_str_status, O_RDONLY);

    int l_prog = strlen(prog);
    write(fd_pid, &l_prog, sizeof(int));
    write(fd_pid, prog, (l_prog+1)*sizeof(char));
    int l_pids = strlen(pids);
    write(fd_pid, &l_pids, sizeof(int));
    write(fd_pid, pids, (l_pids+1)*sizeof(char));

    close(fd_pid);

    int rep;
    read(fd_status, &rep, sizeof(int));
    printf("%s was executed %d times\n", prog, rep);

    close(fd_status);
    unlink(pid_str_status);
    unlink(pid_str_fifo);
}

void stats_uniq (char* pids, int fd_w){
    int bytes;
    pid_t pid_pai = getpid();
    char pid_str_fifo[20];
    
    //printf("pid pai %ld\n", pid_pai);
    sprintf(pid_str_fifo, "tmp/%ld", pid_pai);
    //printf("%s\n",pid_str_fifo);
    mkfifo(pid_str_fifo, 0666);

    write(fd_w, &pid_pai, sizeof(pid_t));
    
    int fd_pid;
    fd_pid = open(pid_str_fifo, O_WRONLY);

    if(fd_pid == -1){
        printf("erro a abrir fifo do status cliente\n");
    }

    int mensagem = 15;
    write(fd_pid, &mensagem, sizeof(int)); // mensagem para executar status-uniq

    char pid_str_status[20];
    sprintf(pid_str_status, "tmp/%ld25", pid_pai);
    int pid_status = atoi(pid_str_status);
    mkfifo(pid_str_status, 0666);
    write(fd_pid, &pid_pai, sizeof(int));
    int fd_status = open(pid_str_status, O_RDONLY);

    int l_pids = strlen(pids);
    write(fd_pid, &l_pids, sizeof(int));
    write(fd_pid, pids, (l_pids+1)*sizeof(char));

    close(fd_pid);

    int rep;
    read(fd_status, &rep, sizeof(int));

    for (int i = 0; i<rep; i++){
        int l;
        read(fd_status, &l, sizeof(int));
        char prog[l+1];
        read(fd_status, prog, (l+1)*sizeof(char));
        printf("%s\n", prog);
    }
    unlink(pid_str_fifo);
}


int main(int argc, char *argv[]) {
    
    int fd = open("tmp/fifo", O_WRONLY);

    if (argc < 2){
        printf("não tem argumentos suficientes\n");
        return 0;
    }

    if (strcmp(argv[1], "execute") == 0) {
        char command[100]; 
        
        strcpy(command, argv[2]); 

        for (int i = 3; i < argc; i++) {
            strcat(command, " ");
            strcat(command, argv[i]); 
        } 
        
        execute(command, strlen(command), fd);
    }
    else if (strcmp(argv[1], "status") == 0) {
        status(fd);
    }
    else if (strcmp (argv[1],"stats-time")==0){
        char command[100];
        strcpy(command, argv[2]);
        for (int i = 3; i < argc; i++) {
            strcat(command, " ");
            strcat(command, argv[i]); 
        }
        //printf("comando: %s\n", command);
        stats_time(fd, command);
    }
    else if (strcmp (argv[1],"stats-comand")==0){
        char program[100];
        strcpy(program, argv[2]);
        char command[100];
        strcpy(command, argv[3]);
        for (int i = 4; i < argc; i++) {
            strcat(command, " ");
            strcat(command, argv[i]); 
        }
        status_comand(program, command, fd);
    }
    else if(strcmp(argv[1], "stats-uniq") == 0){
        char command[100];
        strcpy(command, argv[2]);
        for (int i = 3; i < argc; i++) {
            strcat(command, " ");
            strcat(command, argv[i]); 
        }
        stats_uniq(command, fd);
    }
    else {
        printf("Comando inválido\n");
    }
    
    return 0;
}


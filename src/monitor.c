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
#include "Lista_ligada.h"
#include "monitor_func.h"

#define FREE(algo) if (algo) free(algo)


int main(int argc, char *argv[]) {
    
    int fd_r;

    // check if the fifo exists
    if((fd_r = open("tmp/fifo", O_RDONLY)) == -1) {
        // create the fifo
        if(mkfifo("tmp/fifo", 0666) == -1){
            perror("error fifo 1");
        }
        fd_r = open("tmp/fifo", O_RDONLY);
    }
  
    // open the fifo in read and write mode
    int fd_w = open("tmp/fifo", O_WRONLY);
    if(fd_w == -1) {
        perror("error opening fifo 2");
        return 1;
    }

    int bytes;
    int buffer;
    pid_t fifo_pid;
    LIGADA lista = initLista();

    while ((bytes = read(fd_r, &fifo_pid, sizeof(pid_t))) > 0){
        
        printf("pid do fifo: %ld\n", fifo_pid);
        
        //printf("\n\nlista ligada: \n");
        //pritnLista(lista);

        char pid_str_fifo[20];
        sprintf(pid_str_fifo, "tmp/%ld", fifo_pid);
        int fd_fifo_Pid = open(pid_str_fifo, O_RDONLY);
        if(fd_fifo_Pid == -1) {
            perror("error opening fifo pid in monitor pai");
            return 1;
        }            
        read(fd_fifo_Pid, &buffer, sizeof(int));
        printf("buffer: %d\n", buffer);
        
        
        if(buffer==10){
            executeU(fd_fifo_Pid, &lista);
            //pritnLista(lista);
            close(fd_fifo_Pid);
            
        } else if(buffer==11){
            executeU_final(fd_fifo_Pid, &lista, argv[1]);
            close(fd_fifo_Pid);
            //printf("\n\nlista execute_final: \n");
            //pritnLista(lista);
            unlink(pid_str_fifo);
        } else{
        
            if(fork()== 0){
            
                if(buffer==12){
                    int pid_status;
                    char pid_str_status[20];
                    printf("status\n");
                    read(fd_fifo_Pid, &pid_status, sizeof(int));
                    sprintf(pid_str_status, "tmp/%ld25", pid_status);
                    int fd_status = open(pid_str_status, O_WRONLY);
                    if(fd_status == -1) {
                        perror("error opening fifo pid");
                        return 1;
                    }
                    status(fd_status, &lista);
                    close(fd_status);
                    close(fd_fifo_Pid);
                    unlink(pid_str_fifo);
                }
                else if(buffer == 13){
                    int pid_status_time;
                    char pid_str_statustime[20];
                    printf("status-time\n");
                    read(fd_fifo_Pid, &pid_status_time, sizeof(int));
                    sprintf(pid_str_statustime, "tmp/%ld25", pid_status_time);
                    int fd_status_time = open(pid_str_statustime, O_WRONLY);
                    if(fd_status_time == -1) {
                        perror("error opening fifo pid 1");
                        return 1;
                    }
                    int l;
                    int bytes = read(fd_fifo_Pid, &l, sizeof(int));
                    //printf("bytes: %d\n", bytes);
                    //printf("l: %d\n", l);
                    char comando[l];
                    read(fd_fifo_Pid, comando, (l+1)*sizeof(char));
                    //printf("comando status: %s\n", comando);
                    status_time(comando, fd_status_time, l);
                    close(fd_fifo_Pid);
                    unlink(pid_str_fifo);
                }
                else if(buffer == 14){
                    int pid_status_comand;
                    char pid_str_statuscomand[20];
                    printf("stats-command\n");
                    read(fd_fifo_Pid, &pid_status_comand, sizeof(int));
                    sprintf(pid_str_statuscomand, "tmp/%ld25", pid_status_comand);
                    int fd_status_comand = open(pid_str_statuscomand, O_WRONLY);
                    if(fd_status_comand == -1) {
                        perror("error opening fifo pid");
                        return 1;
                    }
                    
                    int l_prog;
                    read(fd_fifo_Pid, &l_prog, sizeof(int));
                    char prog[l_prog];
                    read(fd_fifo_Pid, prog, (l_prog+1)*sizeof(char));
                    int l_pids;
                    read(fd_fifo_Pid, &l_pids, sizeof(int));
                    char pids[l_pids];
                    read(fd_fifo_Pid, pids, (l_pids+1)*sizeof(char));
                    
                    status_comand(prog, pids, l_pids, fd_status_comand);
                    
                    close(fd_fifo_Pid);
                    unlink(pid_str_fifo);
                }
                else if(buffer == 15){
                    int pid_status_uniq;
                    char pid_str_statusuniq[20];
                    printf("stats-command\n");
                    read(fd_fifo_Pid, &pid_status_uniq, sizeof(int));
                    sprintf(pid_str_statusuniq, "tmp/%ld25", pid_status_uniq);
                    int fd_status_uniq = open(pid_str_statusuniq, O_WRONLY);
                    if(fd_status_uniq == -1) {
                        perror("error opening fifo pid");
                        return 1;
                    }

                    int l_pids;
                    read(fd_fifo_Pid, &l_pids, sizeof(int));
                    char pids[l_pids];
                    read(fd_fifo_Pid, pids, (l_pids+1)*sizeof(char));

                    status_uniq(pids, l_pids, fd_status_uniq);

                    close(fd_fifo_Pid);
                    unlink(pid_str_fifo);
                }
                else{
                    perror("não leu nada");
                }
            }   
        }  
    }  
    close(fd_r);
    close(fd_w);

    return 0;
}

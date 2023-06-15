#ifndef MONITOR_FUNC_H
#define MONITOR_FUNC_H

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


void executeU (int fd_fifo_Pid, LIGADA *lista);
void executeU_final(int fd_fifo_Pid, LIGADA *lista, char *path);
void status(int fd_fifo_Pid, LIGADA* lista);
void status_time (char* pids, int fd_fifo_pid, int l);
void status_comand (char* prog, char* pids, int l_pids, int fd_fifo_pid);

#endif
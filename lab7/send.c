#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int shm_id;

void delete_shm(){
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
}

int main(){
    signal(SIGTERM, delete_shm);
    signal(SIGINT, delete_shm);

    key_t shm_key = 1111;
    shm_id = shmget(shm_key, 512, 0644 | IPC_CREAT | IPC_EXCL);
    if(shm_id < 0){
        fprintf(stderr, "Ошибка: Эта программа уже запущена\n");
        return 1;
    }

    char *shm_ptr = (char*)shmat(shm_id, NULL, 0);
    if(shm_ptr == (char*)-1){
        fprintf(stderr, "Ошибка: %s (%d)\n", strerror(errno), errno);
        return 1;
    }


    time_t cur_time;
    struct tm *cur_time_tm;
    char str_time[64];
    char buf[128];

    while(1){
        cur_time = time(NULL);
        cur_time_tm = localtime(&cur_time);
        strftime(str_time, 64, "%H:%M:%S", cur_time_tm);
    
        snprintf(buf, 128, "Sender. PID: %d. Time: %s", getpid(), str_time);
        
        memcpy(shm_ptr, buf, 128);
        sleep(7);
    }

    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}
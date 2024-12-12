#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int shm_id;

union semum{
    int val;
};

void delete_shm(){
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
}

int main(){
    signal(SIGINT, delete_shm);

    key_t shm_key = ftok("send.c", 14);
    shm_id = shmget(shm_key, 512, 0777 | IPC_CREAT | IPC_EXCL);
    if(shm_id < 0){
        fprintf(stderr, "Ошибка: Эта программа уже запущена\n");
        return 1;
    }

    char *shm_ptr = (char*)shmat(shm_id, NULL, 0);
    if(shm_ptr == (char*)-1){
        fprintf(stderr, "Ошибка: %s (%d)\n", strerror(errno), errno);
        return 1;
    }


    int sem_key = ftok("recieve.c", 13);
    int sem_id = semget(sem_key, 1, 0777 | IPC_CREAT);
    if(sem_id < 0){
        fprintf(stderr, "Ошибка: не удалось создать семафор\n");
        return 1;
    }

    union semum sem_args;
    sem_args.val = 1;
    semctl(sem_id, 0, SETVAL, sem_args);


    time_t cur_time;
    struct tm *cur_time_tm;
    char str_time[64];
    char buf[128];
    struct sembuf sem_buf;

    sem_buf.sem_num = 0;
    sem_buf.sem_flg = 0;

    while(1){
        cur_time = time(NULL);
        cur_time_tm = localtime(&cur_time);
        strftime(str_time, 64, "%H:%M:%S", cur_time_tm);
        snprintf(buf, 128, "Sender PID: %d. Time: %s", getpid(), str_time);
        
        sem_buf.sem_op = -1;
        semop(sem_id, &sem_buf, 1);

        memcpy(shm_ptr, buf, 128);
      
        sem_buf.sem_op = 1;
        semop(sem_id, &sem_buf, 1);
    }

    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    return 0;
}
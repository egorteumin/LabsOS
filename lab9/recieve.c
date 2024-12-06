#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

union semum{
    int val;
};

int main(){
    key_t shm_key = ftok("send.c", 13);
    int shm_id = shmget(shm_key, 512, 0777);
    if(shm_id < 0){
        fprintf(stderr, "Ошибка: %s (%d)\n", strerror(errno), errno);
    }

    char *shm_ptr = (char*)shmat(shm_id, NULL, 0);
    if(shm_ptr == (char*)-1){
        fprintf(stderr, "Ошибка: %s (%d)\n", strerror(errno), errno);
        return 1;
    }


    int sem_key = ftok("recieve.c", 13);
    int sem_id = semget(sem_key, 1, 0777);
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
        snprintf(buf, 128, "Reciver PID: %d. Time: %s", getpid(), str_time);

        sem_buf.sem_op = -1;
        semop(sem_id, &sem_buf, 1);

        printf("%s.\tRecieved string: \"%s\"\n", buf, shm_ptr);
        
        sem_buf.sem_op = 1;
        semop(sem_id, &sem_buf, 1);
        
        sleep(1);
    }

    shmdt(shm_ptr);
    return 0;
}
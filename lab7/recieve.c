#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int main(){
    key_t shm_key = 1111;
    int shm_id = shmget(shm_key, 512, 0644);
    if(shm_id < 0){
        fprintf(stderr, "Ошибка: %s (%d)\n", strerror(errno), errno);
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

        snprintf(buf, 128, "Reciver. PID: %d. Time: %s", getpid(), str_time);

        printf("%s. Recieved string: %s\n", buf, shm_ptr);
        sleep(7);
    }

    shmdt(shm_ptr);
    return 0;
}
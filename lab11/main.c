#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void* write_arr(void *arg){
    size_t n = 0;
    while(1){
        pthread_rwlock_wrlock(&rwlock);
        snprintf(arg, 32, "%ld", n++);
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    pthread_exit(NULL);
}

void* read_arr(void *arg){
    while(1){
        pthread_rwlock_rdlock(&rwlock);
        printf("tid: %lx\tarr: %s\n", pthread_self(), (char*)arg);
        pthread_rwlock_unlock(&rwlock);
        sleep(2);
    }
    pthread_exit(NULL);
}

int main(){
    pthread_t thread_id[11];
    char arr[32];

    pthread_create(&thread_id[0], NULL, write_arr, arr);
    for(int i = 1; i < 11; ++i){
        pthread_create(&thread_id[i], NULL, read_arr, arr);
    }

    for(int i = 0; i < 11; ++i){
        pthread_join(thread_id[i], NULL);
    }
    return 0;
}
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;
int num = 0;

void* read_arr(void *arg){
    while(1){
        pthread_mutex_lock(&mutex);
        printf("tid: %ld\tarray: %s\n", pthread_self(), (char*)arg);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

void* write_arr(void *arg){
    while(1){
        pthread_mutex_lock(&mutex);
        snprintf(arg, 10, "%d", num++);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

int main(){
    char numbers[10];
    pthread_t threads_id[11];

    pthread_mutex_init(&mutex, NULL);
    pthread_create(&threads_id[0], NULL, write_arr, numbers);
    for(int i = 1; i < 11; ++i){
        pthread_create(&threads_id[i], NULL, read_arr, numbers);
    }

    pthread_join(threads_id[0], NULL);
    for(int i = 1; i < 11; ++i){
        pthread_join(threads_id[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}
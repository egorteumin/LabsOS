#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
bool is_ready = false;

void* read_arr(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        while(!is_ready){
            pthread_cond_wait(&cond_var, &mutex);
        }
        is_ready = false;
        printf("tid: %lx\tarr: %s\n", pthread_self(), (char*)arg);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void* write_arr(void* arg){
    size_t n = 0;
    while(1){
        pthread_mutex_lock(&mutex);
        snprintf(arg, 32, "%ld", n++);
        is_ready = true;
        pthread_cond_broadcast(&cond_var);
        pthread_mutex_unlock(&mutex);
        sleep(1);
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
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&mutex);
    return 0;
}
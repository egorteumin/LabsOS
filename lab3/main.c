#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

void atexitFunc(){
    printf("Atexit function in procces %d\n", getpid());
    return;
}

void sigintHandler(int sig){
    printf("Terminal interrupt signal (%d)\n", sig);
    return;
}

void sigtermHandler(int sig){
    printf("Termination signal (%d)\n", sig);
    return;
}


int main(){
    if(atexit(atexitFunc) != 0){
        fprintf(stderr, "Cannot set exit function\n");
        return 1;
    }

    if(signal(SIGINT, sigintHandler) == SIG_ERR){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return 1;
    }
    
    struct sigaction act;
    act.sa_handler = sigtermHandler;

    if(sigaction(SIGTERM, &act, NULL) == -1){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    pid_t res = 0;
    switch(res = fork()){
        case -1:
        {
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            return 1;
            break;
        }
        case 0:
        {
            printf("Child process. Child(my) pid: %d. Parent pid: %d\n", getpid(), getppid());
            sleep(1000);
            break;
        }
        default:
        {
            printf("Parent process. Parent(my) pid: %d. Child pid: %d\n", getpid(), res);
            int a;
            wait(&a);
            printf("Exit status in child: %d\n", WEXITSTATUS(a));
            break;
        }
    }

    return 0;
}
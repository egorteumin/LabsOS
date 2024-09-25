#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int main(){
    __pid_t res = 0;
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
            break;
        }
        default:
        {
            printf("Parent process. Parent(my) pid: %d. Child pid: %d\n", getpid(), res);
            break;
        }
    }

    return 0;
}
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BUF_SIZE 64

int main(){
    int pipe_fd[2];
    if(pipe(pipe_fd) != 0){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    pid_t res = fork();
    switch(res){
        case -1:
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            return 1;
        case 0:
        {
            char buf[BUF_SIZE];
            ssize_t n = 0;
            
            close(pipe_fd[1]);
            while((n = read(pipe_fd[0], buf, BUF_SIZE)) > 0){
                printf("%s", buf);
            }
            close(pipe_fd[0]);

            sleep(5);
            time_t cur_time = time(NULL);
            struct tm *cur_time_tm = localtime(&cur_time);
            char time_str[16];
            strftime(time_str, 16, "%H:%M:%S", cur_time_tm);

            printf("Child time: %s\n", time_str);
            break;
        }
        default:
        {
            time_t cur_time = time(NULL);
            struct tm *cur_time_tm = localtime(&cur_time);
            char time_str[16];
            strftime(time_str, 16, "%H:%M:%S", cur_time_tm);

            char buf[BUF_SIZE];
            snprintf(buf, BUF_SIZE, "PID = %d. Parent time = %s\n", getpid(), time_str);

            close(pipe_fd[0]);
            write(pipe_fd[1], buf, strlen(buf)+1);
            close(pipe_fd[1]);

            int a;
            wait(&a);
            break;
        }
    }

    return 0;
}
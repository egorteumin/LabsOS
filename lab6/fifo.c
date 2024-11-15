#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define BUF_SIZE 64

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "Ошибка: не передано имя для fifo\n");
        return 1;
    }

    if(mkfifo(argv[1], S_IRUSR | S_IWUSR) != 0){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return -1;
    }

    pid_t res = fork();
    switch(res){
        case -1:
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            return 1;
        case 0:
        {
            int fifo_fd = 0;
            if((fifo_fd = open(argv[1], O_RDONLY)) < 0){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }

            char buf[BUF_SIZE];
            ssize_t n = 0;
            while((n = read(fifo_fd, buf, BUF_SIZE)) > 0){
                printf("%s", buf);
            }
            close(fifo_fd);
            
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

            int fifo_fd = 0;
            if((fifo_fd = open(argv[1], O_WRONLY)) < 0){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }
            
            write(fifo_fd, buf, strlen(buf)+1);
            close(fifo_fd);

            int a;
            wait(&a);
            break;
        }
    }

    unlink(argv[1]);
    return 0;
}
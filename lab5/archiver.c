#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 512

enum Flags{
    i = 1,
    e = 2,
    s = 4,
    h = 8
};

struct fmeta{
    char name[256];
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    time_t atime;
    time_t mtime;
};

void insert_file(int archive_fd, const char *file_name){
    int file_fd = 0;
    if((file_fd = open(file_name, O_RDONLY)) == -1){
        fprintf(stderr, "File '%s' cannot be added to archive: %s (%d)\n", file_name, strerror(errno), errno);
        return;
    }

    struct stat file_stat;
    if(fstat(file_fd, &file_stat) == -1){
        fprintf(stderr, "File '%s' cannot be added to archive: %s (%d)\n", file_name, strerror(errno), errno);
        close(file_fd);
        return;
    }

    struct fmeta file_meta;
    strcpy(file_meta.name, file_name);
    file_meta.mode = file_stat.st_mode;
    file_meta.uid = file_stat.st_uid;
    file_meta.gid = file_stat.st_gid;
    file_meta.size = file_stat.st_size;
    file_meta.atime = file_stat.st_atime;
    file_meta.mtime = file_stat.st_mtime;

    if(write(archive_fd, &file_meta, sizeof(file_meta)) == sizeof(file_meta)){
        fprintf(stderr, "File '%s' cannot be added to archive: %s (%d)\n", file_name, strerror(errno), errno);
        close(file_fd);
        return;
    }

    char buf[BUF_SIZE];
    ssize_t n = 0;
    while((n = read(file_fd, buf, BUF_SIZE)) > 0){
        if(write(archive_fd, buf, n) == -1){
            fprintf(stderr, "File '%s' cannot be added to archive: %s (%d)\n", file_name, strerror(errno), errno);
            close(file_fd);
            return;
        }
    }

    close(file_fd);
    return;
}

void extract_file(int archive_fd, const char *file_name){
    struct fmeta file_meta;
    int file_fd;
    char buf[BUF_SIZE];
    ssize_t n;
    off_t file_size;

    while(read(archive_fd, &file_meta, sizeof(file_meta)) == sizeof(file_meta)){
        if(strcmp(file_meta.name, file_name) == 0){
            if((file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, file_meta.mode)) == -1){
                fprintf(stderr, "File '%s' cannot be extracted from archive: %s (%d)\n", file_name, strerror(errno), errno);
                return;
            }

            file_size = file_meta.size;
            while((n = read(archive_fd, buf, BUF_SIZE)) > 0){
                if(write(file_fd, buf, n) != n){
                    fprintf(stderr, "File '%s' cannot be extracted from archive: %s (%d)\n", file_name, strerror(errno), errno);
                    close(file_fd);
                    return;
                }
                file_size -= n;
            }

            struct timeval times[2];
            times[0].tv_sec = file_meta.atime;
            times[1].tv_sec = file_meta.mtime;

            fchown(file_fd, file_meta.uid, file_meta.gid);
            utimes(file_name, times);

            close(file_fd);
            return;
        }
        else{
            lseek(archive_fd, file_meta.size, SEEK_CUR);
        }
    }

    fprintf(stderr, "File '%s' were not found in archive\n", file_name);
    return;
}

void help(){
    printf("Usage: ./archiver [ARCHIVE_NAME] [OPTION] [FILE]...\n\n");
    printf("Possible options:\n");
    printf("-i, --input  \tinsert file(s) to archive\n");
    printf("-e, --extract\textract file(s) from archive\n");
    printf("-s, --stat   \tshows current archive status\n");
    printf("-h, --help   \tdispaly this help and exit\n\n");
    printf("Exit status:\n");
    printf(" 0\tif OK,\n");
    printf(" 1\tif minor problems (e.g., cannot read file)\n");
    return;
}

int main(int argc, char **argv){
    char c;
    int flag = 0;

    while((c = getopt(argc, argv, "iesh")) != -1){
        switch(c){
            case 'i':
                flag += i;
                break;
            case 'e':
                flag += e;
                break;
            case 's':
                flag += s;
                break;
            case 'h':
                flag += h;
                break;
            case '?':
                fprintf(stderr, "Error unknown flag (-h for help menu)\n");
                return 1;
            default:
                break;
        }
    }

    switch(flag){
        case i:
        {
            if(argc < 4){
                fprintf(stderr, "Error: too few arguments (-h for help menu)\n");
                return 1;
            }

            int archive_fd = 0;
            if((archive_fd = open(argv[1], O_WRONLY | O_APPEND | O_CREAT, 777)) == -1){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }

            for(int i = 3; i < argc; ++i){
                insert_file(archive_fd, argv[i]);
            }

            close(archive_fd);
            break;
        }
        case e:
        {
            if(argc < 4){
                fprintf(stderr, "Error: too few arguments (-h for help menu)\n");
                return 1;
            }

            int archive_fd = 0;
            if((archive_fd = open(argv[1], O_RDONLY)) == -1){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }

            for(int i = 3; i < argc; ++i){
                extract_file(archive_fd, argv[i]);
            }

            close(archive_fd);
            break;
        }
        case s:
        case h:
            help();
            break;
        default:
            if(flag != 0){
                fprintf(stderr, "Error: work with many flags are not supported (-h for help menu)\n");
            }
            else{
                fprintf(stderr, "Error: no flags are passed (-h for help menu)\n");
            }
            return 1;
    }

    return 0;
}
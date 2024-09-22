#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#define BUF_SIZE 512

#define RED     "\x1b[31m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int main(int argc, char** argv){
    if(argc <= 1){
        fprintf(stderr, "Too few args\n");
        return 1;
    }

    regex_t regex;
    if(regcomp(&regex, argv[1], 0) != 0){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return 1;
    }


    int arrSize = 0;
    char **fileNameArr = malloc(arrSize * sizeof(*fileNameArr));
    for(int i = 2, j = 0; i < argc; ++i){
        if(argv[i][0] != '-'){
            arrSize++;
            fileNameArr = realloc(fileNameArr, arrSize * sizeof(*fileNameArr));
            fileNameArr[j++] = argv[i];
        }
    }
    if(arrSize == 0){
        char buf[BUF_SIZE];
        char *rtrn;
    
        while((rtrn = fgets(buf, BUF_SIZE, stdin))){
            if(rtrn == NULL){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                regfree(&regex);
                free(fileNameArr);
                return 1;
            }

            if(regexec(&regex, buf, 0, NULL, 0) == 0){
                printf("%s", buf);
            }
        }
        return 0;
    }


    char buf[BUF_SIZE];
    char *rtrn;

    for(int i = 0; i < arrSize; ++i){
        FILE *file = fopen(fileNameArr[i], "r");
        if(!file){
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            regfree(&regex);
            free(fileNameArr);
            fclose(file);
            return 1;
        }

        while((rtrn = fgets(buf, BUF_SIZE, file))){
            if(rtrn == NULL){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                regfree(&regex);
                free(fileNameArr);
                fclose(file);
                return 1;
            }

            if(regexec(&regex, buf, 0, NULL, 0) == 0){
                if(arrSize > 1){
                    printf("%s%s%s:%s", MAGENTA, fileNameArr[i], CYAN, RESET);
                }
                printf("%s", buf);
            }
        }

        fclose(file);
    }

    regfree(&regex);
    free(fileNameArr);
    return 0;
}
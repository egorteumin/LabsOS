#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char** argv){
    if(argc <= 1){
        fprintf(stderr, "Too few args\n");
        return 1;
    }

    bool nFlag = false;
    bool bFlag = false;
    bool EFlag = false;
    char c;
    while((c = getopt(argc, argv, "nbE")) != -1){
        switch(c){
            case 'n':
            {
                nFlag = true;
                break;
            }
            case 'b':
            {
                bFlag = true;
                break;
            }
            case 'E':
            {
                EFlag = true;
                break;
            }
            case '?':
            {
                return 1;
                break;
            }
            default:
            {
                break;
            }
        }
    }


    int arrSize = 0;
    char **fileNameArr = malloc(arrSize * sizeof(*fileNameArr));
    for(int i = 1, j = 0; i < argc; ++i){
        if(argv[i][0] != '-'){
            arrSize++;
            fileNameArr = realloc(fileNameArr, arrSize * sizeof(*fileNameArr));
            fileNameArr[j++] = argv[i];
        }
    }
    if(arrSize == 0){
        free(fileNameArr);
        fprintf(stderr, "No file passed\n");
        return 1;
    }


    for(int i = 0; i < arrSize; ++i){
        FILE *file = fopen(fileNameArr[i], "r");
        if(!file){
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            free(fileNameArr);
            return 1;
        }
        printf("%s:\n", fileNameArr[i]);

        int n;
        size_t nLine = 0;
        if(bFlag){
            nFlag = false;
        }

        char prevCh = '\n';
        while((n = fgetc(file)) != EOF){
            if(bFlag && prevCh == '\n' && n != '\n'){
                printf("    %ld  ", ++nLine);
                prevCh = n;
            }
            else if(nFlag && prevCh == '\n'){
                printf("    %ld  ", ++nLine);
            }
            
            if(EFlag && n == '\n'){
                printf("$");
            }
            printf("%c", n);

            prevCh = n;
        }

        if(arrSize > 1){
            printf("\n");
        }

        fclose(file); 
    }

    free(fileNameArr);
    return 0;
}
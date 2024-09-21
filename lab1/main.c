#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>

#define RESET   "\x1B[0m"
#define GREEN   "\x1B[32m"
#define BLUE    "\x1B[34m"
#define CYAN    "\x1B[36m"
#define RED     "\x1b[31m"

//  Функция для проверки является ли сущность скрытой
int isntHidden(const struct dirent* entity){
    return strncmp(entity->d_name, ".", 1);
}

//  Функция для перевода прав доступа в сторчный вид
void modeToStr(char *str, const __mode_t entityMode){
    switch(entityMode & __S_IFMT){
        case __S_IFSOCK:
        {
            strcpy(str, "s");
            break;
        }
        case __S_IFLNK:
        {
            strcpy(str, "l");
            break;
        }
        case __S_IFREG:
        {
            strcpy(str, "-");
            break;
        }
        case __S_IFBLK:
        {
            strcpy(str, "b");
            break;
        }
        case __S_IFDIR:
        {
            strcpy(str, "d");
            break;
        }
        case __S_IFCHR:
        {
            strcpy(str, "c");
            break;
        }
        case __S_IFIFO:
        {
            strcpy(str, "p");
            break;
        }
        default:
        {
            strcpy(str, "?");
            break;
        }
    }

    __mode_t masks[9] = {S_IRUSR, S_IWUSR, S_IXUSR,
                         S_IRGRP, S_IWGRP, S_IXGRP,
                         S_IROTH, S_IWOTH, S_IXOTH};
    for(int i = 0; i < 9; ++i){
        switch(entityMode & masks[i]){
            case S_IRUSR:
            case S_IRGRP:
            case S_IROTH:
            {
                strcat(str, "r");
                break;
            }
            case S_IWUSR:
            case S_IWGRP:
            case S_IWOTH:
            {
                strcat(str, "w");
                break;
            }
            case S_IXUSR:
            case S_IXGRP:
            case S_IXOTH:
            {
                strcat(str, "x");
                break;
            }
            default:
            {
                strcat(str, "-");
                break;
            }
        }
    }
    return;
}

//  Функция для выбора цвета сущности на которую указывает ссылка
char* linkColor(const char* linkMode){
    switch(linkMode[0]){
        case 'l':
        {
            return CYAN;
            break;
        }
        case 'd':
        {
            return BLUE;
            break;
        }
        case '-':
        {
            if(linkMode[3] == 'x'){
                return GREEN;
            }
            else{
                return RESET;
            }
            break;
        }
        default:
        {
            return RESET;
            break;
        }
    }
    return RESET;
}

size_t totalBlks(const char* dirPath, struct dirent **namelist, const int numEtities){
    struct stat entityInfo;
    char entityPath[256];
    int k = -1;
    size_t total = 0;

    while(++k < numEtities){
        strcpy(entityPath, dirPath);
        strcat(entityPath, "/");
        strcat(entityPath, namelist[k]->d_name);

        if(lstat(entityPath, &entityInfo) == -1){
            return 1;
        }
        else{
            total += entityInfo.st_blocks;
        }
    }

    return total;
}

//  Функция для печати полной информации о директории
int printFullEnt(const char* dirPath, struct dirent **namelist, const int numEtities){
    struct stat entityInfo;
    char entityPath[256];
    int k = -1;

    while(++k < numEtities){
        strcpy(entityPath, dirPath);
        strcat(entityPath, "/");
        strcat(entityPath, namelist[k]->d_name);
    
        if(lstat(entityPath, &entityInfo) == -1){
            return 1;
        }
        else{
            char mode[11];
            modeToStr(mode, entityInfo.st_mode);

            char *userName;
            char *groupName;

            if(getpwuid(entityInfo.st_uid) == NULL){
                sprintf(userName, "%d", entityInfo.st_uid);
            }
            else{
                userName = getpwuid(entityInfo.st_uid)->pw_name;
            }

            if(getgrgid(entityInfo.st_gid) == NULL){
                sprintf(groupName, "%d", entityInfo.st_uid);
            }
            else{
                groupName = getgrgid(entityInfo.st_gid)->gr_name;
            }



            char modTimeStr[64];
            char filter[10] = "%b %e";
            time_t modTime = entityInfo.st_mtime;
            time_t now = time(NULL);
            if(difftime(now, modTime) >= 15552000){
                strcat(filter, "  %Y");
            }
            else{
                strcat(filter, " %R");
            }
            strftime(modTimeStr, sizeof(modTimeStr), filter, localtime(&modTime));

            //  Определение цвета для сущности. Выглядит ужасно, но работает
            char entColor[6];
            char *linkCol;
            char *linkPath = calloc(256, sizeof(char));
            bool isLink = false;
            switch(mode[0]){
                case 'l':
                {
                    if(readlink(entityPath, linkPath, 256 * sizeof(char)) == -1){
                        free(linkPath);
                        return 1;
                    }

                    isLink = true;
                    
                    char *fullLinkPath = calloc(256, sizeof(char));
                    strcpy(fullLinkPath, dirPath);
                    strcat(fullLinkPath, "/");
                    strcat(fullLinkPath, linkPath);

                    struct stat linkStat;
                    if(lstat(fullLinkPath, &linkStat) == -1){
                        strcpy(entColor, RED);
                        linkCol = RED;
                        free(fullLinkPath);
                        break;
                    }
                    else{
                        char linkMode[11];
                        modeToStr(linkMode, linkStat.st_mode);
                        linkCol = linkColor(linkMode);
                    }

                    strcpy(entColor, CYAN);
                    free(fullLinkPath);
                    break;
                }
                case 'd':
                {
                    strcpy(entColor, BLUE);
                    break;
                }
                case '-':
                {
                    if(mode[3] == 'x'){
                        strcpy(entColor, GREEN);
                    }
                    else{
                        strcpy(entColor, RESET);
                    }
                    break;
                }
                default:
                {
                    strcpy(entColor, RESET);
                    break;
                }
            }

            printf("%s %3ld %-10s %-10s %7ld %s %s%s%s", mode, entityInfo.st_nlink, userName, groupName, entityInfo.st_size, modTimeStr, entColor, namelist[k]->d_name, RESET);
            
            if(isLink){
                printf(" -> %s%s%s", linkCol, linkPath, RESET);
            }
            printf("\n");

            free(linkPath);
        }
        free(namelist[k]);
    }
    free(namelist);
    return 0;
}

//  Функция для печати имен сущностей в директории
int printEntNames(const char* dirPath, struct dirent **namelist, const int numEtities){
    int k = -1;
    char entColor[6];
    struct stat entityInfo;
    char entityPath[256];
    while(++k < numEtities){
        strcpy(entityPath, dirPath);
        strcat(entityPath, "/");
        strcat(entityPath, namelist[k]->d_name);

        if(lstat(entityPath, &entityInfo) == -1){
            return 1;
        }
        else{
            switch(entityInfo.st_mode & __S_IFMT){
                case __S_IFLNK:
                {
                    strcpy(entColor, CYAN);
                    break;
                }
                case __S_IFREG:
                {
                    if(entityInfo.st_mode & S_IXUSR){
                        strcpy(entColor, GREEN);
                    }
                    else{
                        strcpy(entColor, RESET);
                    }
                    break;
                }
                case __S_IFDIR:
                {
                    strcpy(entColor, BLUE);
                    break;
                }
                default:
                {
                    strcpy(entColor, RESET);
                    break;
                }
            }
        }

        printf("%s%s%s", entColor, namelist[k]->d_name, RESET);
        if(k < numEtities-1){
            printf("  ");
        }
        free(namelist[k]);
    }
    printf("\n");
    free(namelist);
    return 0;
}

//  Общая функция для печати директории
int printDir(const char *dirPath, int (*filter)(const struct dirent* entity), const bool lFlag){
    struct dirent **namelist;
    int n = scandir(dirPath, &namelist, filter, alphasort);
    if(n == -1){
        return 1;
    }
    else{
        if(lFlag){
            printf("total %ld\n", totalBlks(dirPath, namelist, n)/2);
            return printFullEnt(dirPath, namelist, n);
        }
        else{
            return printEntNames(dirPath, namelist, n);
        }
    }
    return 0;
}

int main(int argc, char** argv){
    bool lFlag = false;
    bool aFlag = false;

    char c;
    while((c = getopt(argc, argv, "la")) != -1){
        switch(c){
            case 'l':
            {
                lFlag = true;
                break;
            }
            case 'a':
            {
                aFlag = true;
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
    char **dirArr = malloc(arrSize * sizeof(*dirArr));
    for(int i = 1, j = 0; i < argc; ++i){
        if(argv[i][0] != '-'){
            arrSize++;
            dirArr = realloc(dirArr, arrSize * sizeof(*dirArr));
            dirArr[j++] = argv[i];
        }
    }
    if(arrSize == 0){
        dirArr = realloc(dirArr, (++arrSize) * sizeof(*dirArr));
        dirArr[0] = ".";
    }

    for(int i = 0; i < arrSize; ++i){
        if(arrSize != 1){
            printf("%s:\n", dirArr[i]);
        }

        if(lFlag & aFlag){
            if(printDir(dirArr[i], NULL, lFlag)){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }
        }
        else if(lFlag & !aFlag){
            if(printDir(dirArr[i], isntHidden, lFlag)){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }
        }
        else if(!lFlag & aFlag){
            if(printDir(dirArr[i], NULL, lFlag)){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }
        }
        else{
            if(printDir(dirArr[i], isntHidden, lFlag)){
                fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
                return 1;
            }
        }

        if(arrSize != 1 && i != arrSize-1){
            printf("\n");
        }
    }

    free(dirArr);
    return 0;
}
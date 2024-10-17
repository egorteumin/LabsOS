#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

void chmod_lidl(const bool is_plus, mode_t *mode, const mode_t new_mode){    
    if(is_plus){
        if(*mode & new_mode){
            return;
        }
        *mode += new_mode;
    }
    else if(!is_plus && *mode & new_mode){
        *mode -= new_mode;
    }
    return;
}

void change_mode(const char perm, const int oct_who, mode_t *mode, const bool is_plus){
    mode_t mask[3];
    switch(perm){
        case 'r':
            mask[0] = S_IRUSR;
            mask[1] = S_IRGRP;
            mask[2] = S_IROTH;
            break;
        case 'w':
            mask[0] = S_IWUSR;
            mask[1] = S_IWGRP;
            mask[2] = S_IWOTH;
            break;
        case 'x':
            mask[0] = S_IXUSR;
            mask[1] = S_IXGRP;
            mask[2] = S_IXOTH;
            break;
        default:
            return;
    }

    switch(oct_who){
        case 01:
            chmod_lidl(is_plus, mode, mask[0]);
            break;
        case 02:
            chmod_lidl(is_plus, mode, mask[1]);
            break;
        case 03:
            chmod_lidl(is_plus, mode, mask[0] + mask[1]);
            break;
        case 04:
            chmod_lidl(is_plus, mode, mask[2]);
            break;
        case 05:
            chmod_lidl(is_plus, mode, mask[0] + mask[2]);
            break;
        case 06:
            chmod_lidl(is_plus, mode, mask[1] + mask[2]);
            break;
        case 0:
        case 07:
            chmod_lidl(is_plus, mode, mask[0] + mask[1] + mask[2]);
            break;
        default:
            break;
    }
}

int main(int argc, char** argv){
    if(argc < 3){
        fprintf(stderr, "Error: no file passed\n");
        return 1;
    }

    long oct_num = 0;
    char *end_oct_str;
    mode_t new_mode = 0;
    char *str_mode = argv[1];
    char *file = argv[2];

    if(str_mode[0] >= '0' && str_mode[0] <= '7'){
        if(strlen(str_mode) > 4){
            fprintf(stderr, "Error: invalid mode: '%s'\n", str_mode);
            return 1;
        }

        oct_num = strtol(str_mode, &end_oct_str, 8);
        
        if(errno || *end_oct_str){
            fprintf(stderr, "Error: invalid mode: '%s'\n", str_mode);
            return 1;
        }

        new_mode = (mode_t)oct_num;
    }
    else{
        if(strlen(str_mode) == 1){
            fprintf(stderr, "Error: invalid mode: '%s'\n", str_mode);
            return 1;
        }

        // Проверяем строку на правильность
        char *allowed = "augo+-";
        for(size_t i = 0; i < strlen(str_mode); ++i){
            if(strchr(allowed, str_mode[i]) != NULL){
                switch(str_mode[i]){
                    case '+':
                    case '-':
                    case 'r':
                    case 'w':
                    case 'x':
                        allowed = "+-rwx";
                        break;
                    case 'a':
                    case 'u':
                    case 'g':
                    case 'o':
                    default:
                        break;
                }
            }
            else{
                fprintf(stderr, "Error: invalid mode: '%s'\n", str_mode);
                return 1;
            }
        }


        struct stat st;
        if(stat(file, &st) == -1){
            fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
            return 1;
        }

        new_mode = st.st_mode;
        bool is_plus = true;
        int who = 0;
        // Вычисляем новые права доступа
        for(size_t i = 0; i < strlen(str_mode); ++i){
            switch(str_mode[i]){
                case 'a':
                    who = 07;
                    break;
                case 'u':
                    if(who % 2 == 0){
                        who += 01;
                    }
                    break;
                case 'g':
                    if(who != 01 || who != 05 || who != 07){
                        who += 02;
                    }
                    break;
                case 'o':
                    if(who < 04){
                        who += 04;
                    }
                    break;
                case '+':
                    is_plus = true;
                    break;
                case '-':
                    is_plus = false;
                    break;
                case 'r':
                    change_mode('r', who, &new_mode, is_plus);
                    break;
                case 'w':
                    change_mode('w', who, &new_mode, is_plus);
                    break;
                case 'x':
                    change_mode('x', who, &new_mode, is_plus);
                    break;
                default:
                    break;
            }
        }

    }

    // Меняем права доступа
    if(chmod(file, new_mode) == -1){
        fprintf(stderr, "Error: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    return 0;
}
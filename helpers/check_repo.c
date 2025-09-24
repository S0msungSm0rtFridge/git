#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#ifdef _WIN32
  #define PATH_SEP '\\'
#else
  #define PATH_SEP '/'
#endif
//check if a directoy exist
static int dir_exist(const char *check_dir){
    struct stat st;
    return ((stat(check_dir, &st) == 0) & S_ISDIR(st.st_mode));
}

//find the paretn dir
static char *get_parent_dir(const char *check_path){

    char *copy = strdup(check_path);
    if(!copy) return NULL;

   
    char *head_dir = strrchr(copy, PATH_SEP);
    if(head_dir){
        if(head_dir == copy){
            copy[1] = '\0';
        }else{
            *head_dir = '\0';
        }
    }
    return copy;
}


//find parent dir to call git funcitons through
char *find_root_repo(const char *start_dir){

    char *dir = strdup(start_dir);
    if(!dir) return NULL;

    while(1){
        size_t len =  strlen(dir) + strlen("./mygit" + 1);
        char *test = malloc(len);
        if(!test){
            free(dir);
            return NULL;
        }

        snprintf(test, len, "%s%c.mygit", dir, PATH_SEP);

        if(dir_exist(test)){
            free(test);
            return dir;
        }

        free(test);
        if (strcmp(dir, "/") == 0  
        
#ifdef _WIN32
 || (strlen(dir) == 2 && dir[1] == ':')
#endif
){
            free(dir);
            return NULL;
        }
        char *parent = get_parent_dir(dir);
        free(dir);
        dir = parent;
        if(!dir) return NULL;
    }
}


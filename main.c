#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <errno.h>
#include "./headers/init.h"
#include <unistd.h> 
#include "./headers/check_repo.h"
#include"./headers/add.h"
#include"./headers/commit.h"
#include"./headers/push.h"


//if linux or windows
#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

//table of alll git funcs, will prob change 
typedef int(*command)(int argc, char *argv[]);
typedef struct {
    const char *name;
    command func;
} Command;

Command commands[] = {
    {"init", mygit_init},
    {"add", mygit_add},
    {"commit", mygit_commit},
    {"push", mygit_push},
    {NULL, NULL}
};


int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Usage: mygit <command>\n");
        return 1;
    }

    //will loop through all commands to find matching, then take argv
    for(int i = 0; commands[i].name != NULL; i++){
        if(strcmp(argv[1], commands[i].name) == 0){
            return(commands[i].func(argc, argv));
        }
    }

    printf("unknown command");
    return 1;
    
    // char cwd[PATH_MAX];

    // get current working directory
    // if (!getcwd(cwd, sizeof(cwd))) {
    //     perror("getcwd");
    //     return 1;
    // }

    // printf("Current directory: %s\n", cwd);

    // // find .mygit root
    // char *root = find_root_repo(cwd);
    // if (root) {
    //     printf("Repo root found at: %s\n", root);
    //     free(root);  // always free strdup-ed memory
    // } else {
    //     printf("Not inside a mygit repository.\n");
    // }

    // return 0;


    return 0;

}

int mygit_init(int argc, char *argv[]){

    if(MKDIR(".mygit") && errno != EEXIST){
        perror("mkdir .mygit");
        return 1;
    }

    //creates the mygit dir with places to store previous commits
    const char *dirs[] = {
        ".mygit/objects",
        ".mygit/refs",
        ".mygit/refs/heads",
        ".mygit/refs/tags",
    };

    for (int i = 0; i < 4; i++) {
        if (MKDIR(dirs[i]) && errno != EEXIST) {
            fprintf(stderr, "Failed to create directory: %s\n", dirs[i]);
            return 1;
        }
    }

    //creates head file pointing to main
    FILE *head = fopen(".mygit/HEAD", "w");
    if (!head) {
        perror("fopen HEAD");
        return 1;
    }
    fprintf(head, "ref: refs/heads/main\n");
    fclose(head);

    //creates a nindex
    FILE *index = fopen(".mygit/index", "w");
    if (!index) {
        perror("index failed");
        return 1;
    }
    fclose(index);

    //creates git configs
    FILE *config = fopen(".mygit/config", "w");
    if (config) {
        fprintf(config, "[core]\n\trepositoryformatversion = 0\n\tfilemode = false\n\tbare = false\n");
        fclose(config);
    }

    printf("Initialized empty mygit repository in .mygit/\n");
    return 0;
}

unsigned char *read_blob(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buf = malloc(size);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t n = fread(buf, 1, size, f);
    fclose(f);

    if (n != size) {
        free(buf);
        return NULL;
    }

    if (out_size)
        *out_size = size;

    return buf;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

int main(int argc, char *argv[]){

    printf("%d", argc);
}


int mygit_init(){

    if(mkdir(".mygit", 0755) && errno != EEXIST){
        perror("mkdir .mygit");
        return 1;
    }

    //creates the mygit dir with places to store previous commits
    const char *dirs[] = {
        ".mygit/objects",
        ".mygit/refs",
        ".mygit/refs/heads",
        ".mygit/refs/tags"
    };

    for (int i = 0; i < 4; i++) {
        if (mkdir(dirs[i], 0755) && errno != EEXIST) {
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

    //creates git configs
    FILE *config = fopen(".mygit/config", "w");
    if (config) {
        fprintf(config, "[core]\n\trepositoryformatversion = 0\n\tfilemode = false\n\tbare = false\n");
        fclose(config);
    }

    printf("Initialized empty mygit repository in .mygit/\n");
    return 0;
}
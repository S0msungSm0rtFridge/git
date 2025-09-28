#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>
  #define mkdir(path, mode) _mkdir(path)
#else
  #include <unistd.h>
#endif

int push_object(const char *hash, const char *dir);
int push_commit(const char *hash, const char *dir);
int push_tree(const char *hash, const char *dir);
int copy_blob(const char *src, const char *dest);


int checkDir(const char *path){
    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    char *p = tmp;
    while(*p){
        if(*p == '/' && p != tmp){
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
        p++;
    }

    mkdir(tmp, 0755);
    return 0;
}

int push_object(const char *hash, const char *dir){
    printf("gets to object\n");

    //get the path of the object
    char path[1024];
    snprintf(path, sizeof(path), ".mygit/objects/%.2s/%s", hash, hash + 2);
    printf("path: %s\n", path);

    //read the entire file
    FILE *f = fopen(path, "rb");
    if(!f){
        printf("fails at opening file object \n");
        return 1;
    }

    char buffer[4096];
    size_t n = fread(buffer, 1, sizeof(buffer), f);
    buffer[n - 1] = '\0';
    fclose(f);

    printf("path: %s\n", path);
    char type[16];
    int size = 0;
    sscanf(buffer, "%15s %d", type, &size);

    printf("path: %s\n", path);
    //if commit object, go commit, else tree, go tree. else blobl
    if(strcmp(type, "commit") == 0){
        return push_commit(hash, dir);
    }
    else if(strcmp(type, "tree") == 0){
        return push_tree(hash, dir);
    } 
    else{
        printf("in copy\n");
        printf("path: %s\n", path);
        char outDir[128], outFile[256];
        snprintf(outDir, sizeof(outDir), "%s/%.2s", dir, hash);
        mkdir(outDir, 0775);
        snprintf(outFile, sizeof(outFile), "%s/%.2s/%s", dir, hash, hash+2);
        printf("path: %s\n", path);
        return copy_blob(path, outFile);
    }
    return 0;
}

int push_tree(const char *hash, const char *dir) {
    printf("gets to tree %s\n", hash);

    // get the path of the object
    char path[1024];
    snprintf(path, sizeof(path), ".mygit/objects/%.2s/%s", hash, hash + 2);

    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "failed opening tree object\n");
        return 1;
    }

    // read file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    unsigned char *buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    // sjipp header
    unsigned char *pos = buffer;
    pos = (unsigned char *)strchr((char *)buffer, '\0') + 1;

    while (pos < buffer + size) {
        // ended by /0
        char mode[7];
        char filename[256];

        //get moe
        char *sp = strchr((char *)pos, ' ');
        int mode_len = sp - (char *)pos;
        memcpy(mode, pos, mode_len);
        mode[mode_len] = '\0';
        pos = (unsigned char *)sp + 1;

        // get name
        char *nul = (char *)pos;
        while (*nul != '\0') nul++;
        int fname_len = nul - (char *)pos;
        memcpy(filename, pos, fname_len);
        filename[fname_len] = '\0';
        pos = (unsigned char *)nul + 1;

        //skpi 20 for hash
        unsigned char hash_bin[20];
        memcpy(hash_bin, pos, 20);
        pos += 20;

        // convert to 40 string
        char objHash[41];
        for (int i = 0; i < 20; i++) {
            sprintf(objHash + i*2, "%02x", hash_bin[i]);
        }
        objHash[40] = '\0';

        printf("entry: %s %s %s\n", mode, filename, objHash);

        push_object(objHash, dir);
    }

    free(buffer);
    return 0;
}

int push_commit(const char *hash, const char *dir){

    printf("gets to commit, %s, %s\n", hash, dir);
    char path[1024];
    snprintf(path, sizeof(path), ".mygit/objects/%.2s/%s", hash, hash + 2);
    printf("%s\n", path);
    FILE *f = fopen(path, "rb");
    if(!f){
        printf("fails opening file\n");
        return 1;
    }

    //read commit object
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return 1;
    }

    fread(buffer, 1, size, f);
    fclose(f);
    buffer[size] = '\0'; // safety

    // skip past header: "<type> <size>\0"
    char *content = strchr(buffer, '\0');
    if (!content) {
        fprintf(stderr, "Invalid commit object\n");
        free(buffer);
        return 1;
    }
    content++;

    //makes name for location of where to put it
    char outDir[1024];
    char outFile[1024];
    snprintf(outDir, sizeof(outDir), "%s/%.2s", dir, hash);
    snprintf(outFile, sizeof(outFile), "%s/%.2s/%s", dir, hash, hash + 2);

    //finds first occurance of tree in com mit object
    char tree_hash[41] = {0};
    char *tree_line = strstr(content, "tree ");
    printf("%s, %s\n", tree_hash, tree_line);
    if(tree_line){
        sscanf(tree_line, "tree %40s", tree_hash);
        push_object(tree_hash, dir);
    }
    free(buffer);
    return 0;

}

int copy_blob(const char *src, const char *dest){


    char tmp[1024];
    strncpy(tmp, dest, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    char *dir = dirname(tmp);
    checkDir(dir);

    printf("%s, %s\n", src, dest);
    FILE *s = fopen(src, "rb");
    if(!s){
        printf("failed to read in file");
        return 1;
    }

    FILE *out = fopen(dest, "w");
    if(!out){
        printf("failed to make out file");
        return 1;
    }

    char buffer[4096];
    size_t n; 
    while((n = fread(buffer, 1, sizeof(buffer), s)) > 0){
        fwrite(buffer, 1, n, out);
    }
    
    fclose(s);
    fclose(out);
    return 0;

}


//making fake push with only local storage rn not remote
int mygit_push(int argc, char *argv[]){
    printf("inital runs");
    if(argc > 2){
        return 1;
    }

    FILE *f = fopen(".mygit/HEAD", "r");
    if (!f) {
        fprintf(stderr, "No HEAD found\n");
        return 1;
    }

    char head_hash[41];
    if (!fgets(head_hash, sizeof(head_hash), f)) {
        fclose(f);
        fprintf(stderr, "Empty HEAD\n");
        return 1;
    }
    fclose(f);

    head_hash[strcspn(head_hash, "\n")] = 0;

    return push_object(head_hash, ".mygit/remote/objects");
}

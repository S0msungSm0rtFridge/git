#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

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


int push_object(const char *hash, const char *dir){
    printf("gets to object");

    //get the path of the object
    char path[1024];
    snprintf(path, sizeof(path), ".mygit/objects/%.2s/%s", hash, hash + 2);
    printf("%s\n", path);

    //read the entire file
    FILE *f = fopen(path, "rb");
    if(!f){
        return 1;
    }

    char buffer[4096];
    size_t n = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    printf("%s\n", buffer);

    printf("before token\n");
    char *copyString = strdup(buffer);
    char *token = strtok(copyString, " ");
    printf("after token, %s\n", token);

    //if commit object, go commit, else tree, go tree. else blobl
    if(strcmp(token, "commit") == 0){
        return push_commit(hash, dir);
    }
    else if(strcmp(token, "tree") == 0){
        return push_tree(hash, dir);
    } 
    else{
        printf("in copy\n");
        char outDir[128], outFile[256];
        snprintf(outDir, sizeof(outDir), "%s/%.2ss", dir, hash);
        snprintf(outFile, sizeof(outFile), "%s/%.2ss/%s", dir, hash, hash+2);
        return copy_blob(path, outFile);
    }
    free(copyString);
    return 0;
}

int push_tree(const char *hash, const char *dir){
    printf("gets to tree");
    //get the path of the object
    char path[1024];
    snprintf(path, sizeof(path), ".mygit/objects/%.2s/%s", hash, hash + 2);
    printf("%s\n", path);


    //read the entire file
    FILE *f = fopen(path, "rb");
    if(!f){
        printf("failed in poening file\n");
        return 1;
    }

    char buffer[4096];
    size_t n = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    printf("buffer %s\n", buffer);

    // wlk tree"100644 filename\0<20-byte hash>"
    size_t pos = 0;
    while (pos < n) {
        char mode[7], filename[256], objHash[41];
        unsigned char hashBin[20];

        // mode + space
        sscanf((char*)buffer + pos, "%s", mode);
        char *fname = strchr((char*)buffer + pos, ' ') + 1;

        size_t fname_len = strlen(fname);
        pos += strlen(mode) + 1 + fname_len + 1; 

        memcpy(hashBin, buffer + pos, 20);
        pos += 20;

        // turn to string
        for (int i = 0; i < 20; i++) {
            sprintf(objHash + i*2, "%02x", hashBin[i]);
        }
        objHash[40] = '\0';
        printf("%s, %s\n", objHash, dir);
        push_object(objHash, dir);
    }

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

    FILE *s = fopen(src, "rb");
    if(!s){
        return 1;
    }

    FILE *out = fopen(dest, "w");
    if(!out){
        return 1;
    }

    char buffer[4096];
    size_t n; 
    while((n = fread(buffer, 1, sizeof(buffer), s)) > 0){
        fwrite(buffer, 1, sizeof(buffer), out);
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

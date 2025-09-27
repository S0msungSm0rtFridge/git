#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#ifdef _WIN32
  #include <direct.h>
  #define mkdir(path, mode) _mkdir(path)
#else
  #include <sys/types.h>
#endif

// helper: write object to .mygit/objects
char *write_object(const unsigned char *content, size_t size, const char *type) {
    char header[64];
    int header_len = snprintf(header, sizeof(header), "%s %zu", type, size) + 1;
    size_t obj_size = header_len + size;

    unsigned char *obj_buf = malloc(obj_size);
    if(!obj_buf) return NULL;
    memcpy(obj_buf, header, header_len);
    memcpy(obj_buf + header_len, content, size);

    unsigned char hash_bin[20];
    SHA1(obj_buf, obj_size, hash_bin);

    char *hex = malloc(41);
    for(int i=0;i<20;i++)
        sprintf(hex + i*2, "%02x", hash_bin[i]);
    hex[40] = '\0';

    char dir[64], file[64];
    snprintf(dir, sizeof(dir), ".mygit/objects/%.2s", hex);
    snprintf(file, sizeof(file), ".mygit/objects/%.2s/%s", hex, hex+2);
    mkdir(dir, 0755);

    FILE *f = fopen(file, "wb");
    if(f) {
        fwrite(obj_buf, 1, obj_size, f);
        fclose(f);
    }

    free(obj_buf);
    return hex;
}

// main commit function
int commit(const char *message) {
    // make tree
    char mode[] = "100644"; 
    unsigned char tree_buf[4096];
    size_t tree_len = 0;
    char hash_hex[41], path[256];

    FILE *index = fopen(".mygit/index", "r");
    if (!index) {
        printf("No staged files.\n");
        return 1;
    }

    //check to see if nothing added
    fseek(index, 0, SEEK_END);
    long size = ftell(index);
    if (size == 0) {
        printf("Index is empty.\n");
        fclose(index);
        return 1;
    }

    fseek(index, 0, SEEK_SET);

    while(fscanf(index, "%40s %255s", hash_hex, path) == 2) {
        unsigned char hash_bin[20];
        for(int i = 0; i < 20; i++ ){
            sscanf(hash_hex+i*2,"%2hhx",&hash_bin[i]);
        } 

        tree_len += snprintf((char*)tree_buf + tree_len, sizeof(tree_buf)-tree_len,
                             "%s %s", mode, path);
        tree_buf[tree_len++] = 0;
        memcpy(tree_buf + tree_len, hash_bin, 20);
        tree_len += 20;
    }
    fclose(index);

    char *tree_hash = write_object(tree_buf, tree_len, "tree");

    // make commit
    char parent_hash[64] = ""; 
    FILE *head = fopen(".mygit/HEAD", "r");
    if(head) {
        fgets(parent_hash, sizeof(parent_hash), head);
        parent_hash[strcspn(parent_hash, "\n")] = 0; 
        fclose(head);
    }

    char commit_buf[8192];
    size_t len = 0;
    len += snprintf(commit_buf + len, sizeof(commit_buf)-len, "tree %s\n", tree_hash);
    if(parent_hash[0] != '\0'){
        len += snprintf(commit_buf + len, sizeof(commit_buf)-len, "parent %s\n", parent_hash);
    }
    len += snprintf(commit_buf + len, sizeof(commit_buf)-len, "author You <you@example.com>\n");
    len += snprintf(commit_buf + len, sizeof(commit_buf)-len, "message %s\n", message);

    char *commit_hash = write_object((unsigned char*)commit_buf, len, "commit");

    // uopdatev head
    FILE *currHead = fopen(".mygit/HEAD", "w");
    if(currHead) {
        fprintf(currHead, "%s\n", commit_hash);
        fclose(currHead);
    }

    // clear index
    FILE *f = fopen(".mygit/index", "w");
    if(f) fclose(f);

    printf("[commit %s] %s\n", commit_hash, message);
    free(tree_hash);
    free(commit_hash);
    return 0;
}

int mygit_commit(int argc, char *argv[]){

    if(argc < 2){
        return 1;
    }

    if(argc > 2 && strcmp(argv[2], "-m") != 0) {
        return 1;
    }

    if(argc > 4){
        return 1;
    }
    return commit(argv[3]);
}
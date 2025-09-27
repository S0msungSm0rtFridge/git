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
//will returns a hex string of SHA-1 to name the file beiong made blobl
// char *hash_file(const char *path){
//     FILE *f = fopen(path, "rb");
//     if(!f){
//         printf("wtf");
//         return NULL;
//     }

//     SHA_CTX ctx;
//     SHA1_Init(&ctx);
    
//     char buffer[4096];
//     size_t n;

//     while((n = fread(buffer, 1, sizeof(buffer), f)) > 0){
//         SHA1_Update(&ctx, buffer, n);
//     }

//     fclose(f);

//     //make it into a hex string to store hash
//     unsigned char hash[SHA_DIGEST_LENGTH];
//     SHA1_Final(hash, &ctx);

//     char *hex = malloc((SHA_DIGEST_LENGTH * 2) + 1);
//     if(!hex){
//         return NULL;
//     }

//     for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
//         sprintf(hex + (i*2), "%02x", hash[i]);
//     }

//     hex[SHA_DIGEST_LENGTH * 2] = '\0';
//     return hex;
// }
char *blob(const char *path);
int add(int argc, char *argv[]);
int mygit_add(int argc, char *argv[]);

char *blob(const char *path){
    FILE *f = fopen(path, "rb");
    if(!f){
        return NULL;
    }

    //grab everything and get to end for size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);


    unsigned char *buf = malloc(size);
    if(!buf){
        fclose(f);
        return NULL;
    }

    fread(buf, 1, size, f);
    fclose(f);

    char header[64];
    int header_len = snprintf(header, sizeof(header), "blob %zu", size) + 1; 
    size_t blob_size = header_len + size;

    unsigned char *blob = malloc(blob_size);
    memcpy(blob, header, header_len);
    memcpy(blob + header_len, buf, size);
    free(buf);
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(blob, blob_size, hash);

    char *hex = malloc(SHA_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
        sprintf(hex + i*2, "%02x", hash[i]);
    }
    hex[SHA_DIGEST_LENGTH*2] = '\0';

    char dir[64], file[64];
    snprintf(dir, sizeof(dir), ".mygit/objects/%.2s", hex);
    snprintf(file, sizeof(file), ".mygit/objects/%.2s/%s", hex, hex+2);

    mkdir(dir, 0755);

    FILE *out = fopen(file, "wb");
    fwrite(blob, 1, blob_size, out);
    fclose(out);

    free(blob);

    return hex;


}

//need to make sure this works from any path in git repo, use function in check_repo, will also add . in top level
int add(int argc, char *argv[]){

    for (int i = 2; i < argc; i++) {
        char *path = argv[i];
        char *hashed_file = blob(path);

        if(hashed_file == NULL){
            printf("the path %s does not exist", path);
            continue;
        }
        FILE *f = fopen(".mygit/index", "r");
        FILE *tmp = fopen(".mygit/index.tmp", "w");
        if(!f){
            printf("fix init ig");
            free(hashed_file);
            return 1;
        }
        char line[1024];
        while(fgets(line, sizeof(line), f) != NULL){

            //makes sure index is not empty
            line[strcspn(line, "\n")] = 0;
            if (line[0] == '\0') {
                break;
            }

            char *space = strchr(line, ' ');
            char *file_name = space + 1;
            file_name[strcspn(file_name, "\n")] = 0;        

            if(strcmp(file_name, path) == 0){
                continue;
            }

            fputs(line, tmp);
            fputc('\n', tmp);
        }

        fprintf(tmp, "%s %s\n", hashed_file, path);
        fclose(tmp);
        fclose(f);
        free(hashed_file);
        
        if (remove(".mygit/index") != 0) {
        }
        if (rename(".mygit/index.tmp", ".mygit/index") != 0) {
            perror("rename");
            return 1;
        }
    }

    return 0;
}

int mygit_add(int argc, char *argv[]){
    if(argc < 3){
        printf("need file path");
        return 1;
    }

    return add(argc, argv);
}
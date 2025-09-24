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
int add(const char *path);
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


int add(const char *path){
    printf("change");
    char *hashed_file = blob(path);
    FILE *f = fopen(".mygit/index", "a");
    if(!f){
        printf("fix init ig");
        free(hashed_file);
        return NULL;
    }

    fprintf(f, "%s %s", hashed_file, path);
    fclose(f);
    free(hashed_file);
    return 0;
}

int mygit_add(int argc, char *argv[]){
    if(argc < 3){
        return 1;
    }

    return add(argv[2]);
}
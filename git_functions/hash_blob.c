#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

//will returns a hex string of SHA-1 to name the file beiong made blobl
char *hash_file(const char *path){
    FILE *f = fopen(path, "rb");
    if(!f){
        printf("wtf");
        return NULL;
    }

    SHA_CTX ctx;
    SHA1_Init(&ctx);
    
    char buffer[4096];
    size_t n;

    while((n = fread(buffer, 1, sizeof(buffer), f)) > 0){
        SHA1_Update(&ctx, buffer, n);
    }

    fclose(f);

    //make it into a hex string to store hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &ctx);

    char *hex = malloc((SHA_DIGEST_LENGTH * 2) + 1);
    if(!hex){
        return NULL;
    }

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
        sprintf(hex + (i*2), "%02x", hash[i]);
    }

    hex[SHA_DIGEST_LENGTH * 2] = "\0";
    return hex;
}

char *blob_file(const char *path){
    FILE *f = fopen(path, "rb");
    if(!f){
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    
}
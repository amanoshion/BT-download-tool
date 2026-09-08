#ifndef HASH_H
#define HASH_H
#include <sodium.h>
#include <stdlib.h>
#include <stdio.h>

#include "protocol.h"
#define ERROR -1
#define OK 0

// #define MAX_CONNECTED 1024
#define FILE_SIZE 4096
#define DEFAULT_HASH_TABLE_SIZE 256
#define MAX_PEER_NUM_PER_FILE 128
#define HASH_LEN 65

typedef struct FileData {
        int sockfd;
        char filename[S];
        char filepath[M];
} FileData;

typedef struct HashNode {
        FileData filedatas[MAX_PEER_NUM_PER_FILE];
        char hashkey[65];
        struct HashNode *next;
} HashNode;

typedef struct {
        HashNode *head;
        int curr_size;
        int curr_count;
} HashTable;

// TODO : add hashNode
int add_hashNode(HashTable *ht, HashNode *hn) {

}

// TODO : delete_hashNode
// TODO : search_hashNode by hashkey

int realloc_hashTable(HashTable *ht) {
        if (ht->curr_count < 0.8 * (ht->curr_size)) return;

        int oldSize = ht->curr_size;
        int newSize = oldSize * 1.5;

        HashTable *new_table;
        new_table = realloc(ht, newSize * sizeof(HashNode));
        if (new_table == NULL) {
                perror("realloc fail");
                return ERROR;
        } 

        free_hasTable(ht);

        new_table->curr_count = ht->curr_count;
        new_table->curr_size = newSize;

        return OK;
}

void *ini_hashTable(HashTable *ht) {
        // ini head
        memset(ht->hashkey, 0, strlen(ht->hashkey));
        memset(ht->filedatas, 0, sizeof(FileData) * MAX_PEER_NUM_PER_FILE);
        ht->next = NULL;
        ht->curr_size = DEFAULT_HASH_TABLE_SIZE;
        ht->curr_count = 0;
        // ini nodes

        HashNode *curr = ht->head;
        for (int i = 0; i < DEFAULT_HASH_TABLE_SIZE; i++) {
                curr->next = malloc(sizeof(HashNode));
                curr = curr->next;
                memset(curr->hashkey, 0, strlen(curr->hashkey));
                memset(curr->filedatas, 0, sizeof(FileData) * MAX_PEER_NUM_PER_FILE);
                curr->next = NULL;
        }
        return;
}

void free_hasTable(HashTable *ht) {
        HashNode *prev = NULL;
        HashNode *curr = ht->head;
        for (int i = 0; i < ht->curr_size; i++) {
                prev = curr;
                if (curr->next != NULL) {
                        curr = curr->next;
                }
                if (prev != NULL) {
                        free(prev);
                }
        }
        return;
}

void ini_libsodium() {
        if (sodium_init() < 0) {
                fprintf(stderr, "libsodium ini fail\n");
                return EXIT_FAILURE;
        }
}

int file_blake2b(const char *filename, char *hashkey) {  // output len shoud be 65
        FILE *file = fopen(filename, "rb");
        if (!file ) {
                perror("open fail");
                return ERROR;
        }

        unsigned char hash[crypto_generichash_BYTES]; // = 32
        unsigned char buffer[FILE_SIZE] = {0};
        size_t bytes_read;

        crypto_generichash_state state;
        crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);

        while((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                cryptro_generichash_update(&state, buffer, bytes_read);
        }

        if (ferror(file)) {
                perror("read file fail");
                fclose(file);
                return ERROR;
        }

        crypto_generichash_final(&state, hash, crypto_generichash_BYTES);

        fclose(file);

        for (int i = 0; i < crypto_generichash_BYTES; i++) {
                snprintf(hashkey + (i * 2), "02x", hash[i]);
        }

        hashkey[crypto_generichash_BYTES * 2] = '\0';

        return OK;
}       

#ifndef
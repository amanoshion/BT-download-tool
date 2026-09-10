#ifndef HASH_H
#define HASH_H
#include <sodium.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "protocol.h"
#define ERROR -1
#define OK 0

// #define MAX_CONNECTED 1024
#define FILE_SIZE 4096
#define HASH_LEN 65
#define MAX_HASH_TABLE_SIZE 256
#define MAX_PEER_NUM_PER_FILE 128

typedef struct PeerData {
        int sockfd;
        int port;
        char filename[S];
        char filepath[M];
        PeerData *next;
} PeerData;

typedef struct PeerHead {
        PeerData *next;
        int count;
        int max_size;
} PeerHead;

typedef struct HashNode {
        PeerData *peerHead;
        char hashkey[HASH_LEN];
        HashNode *next;
} HashNode;

typedef struct {
        HashNode *next;
        int max_size;
        int count;
} HashHead;

void ini_hash(HashHead *head) {
        // ini head 's basic var
        head->max_size = MAX_HASH_TABLE_SIZE;
        head->count = 0;
        // ini full hashtables by head
        HashNode *curr = head->next;

        PeerData peer_curr = NULL;
        for (int i = 0; i < MAX_HASH_TABLE_SIZE; i++) {
                curr = malloc(sizeof(HashNode));
                // ini PeerData inside HashNode
                peer_curr = curr->peerHead;
                peer_curr = malloc(sizeof(PeerHead));
                peer_curr->count = 0;
                peer_curr->max_size = MAX_PEER_NUM_PER_FILE;
                peer_curr->next = NULL;

                memset(curr->hashkey, 0, HASH_LEN);
                curr = curr->next;
                curr->next = NULL;
        }
        return;
}

void free_hashTable(HashHead *head) {
        HashNode *prev = NULL;
        HashNode *curr = head;
        for (int i = 0; i < head->max_size; i++) {
                prev = curr;
                if (curr->next != NULL) {
                        curr = curr->next;
                }
                if (prev != NULL) {
                        PeerData *peer_prev = NULL;
                        PeerData *peer_curr = curr->peerHead->next;
                        for (int j = 0; j < curr->peerHead->max_size; j++) {
                                peer_prev = peer_curr;
                                if (peer_curr->next != NULL) {
                                        peer_curr = peer_curr->next;
                                }
                                if (prev != NULL) {
                                        free(peer_prev);
                                }
                        }
                        free(curr->peerHead);
                        free(prev);
                }
        }
        return;
}

int add_hashNode(HashHead *head, HashNode *hn) {
        if (head == NULL) return;

        HashNode *entry = head->head;
        HashNode *next = NULL;
        
        if (entry->next != NULL) {
                next = entry->next;
        }

        entry->next = hn;
        hn->next = next;

        head->curr_count++;

        return OK;
}

HashNode *find_hashNode_prev_by_key(HashHead *head, char *hashkey) {
        HashNode *entry = head;
        _Bool isfound = 0;
        while(entry->next != NULL) {
                if (strncmp(entry->next->hashkey, hashkey, HASH_LEN) == 0) {
                        isfound = 1;
                        break;
                } else {
                        entry = entry->next;
                }
        }
        if (isfound == 1) {
                return entry;
        } else {
                return NULL;
        }

}

int delete_hashNode(HashHead *head, char *hashkey) {
        HashNode *entry_prev = find_hashNode_prev_by_key(head, hashkey);
        if (entry_prev == NULL || entry_prev->next == NULL) return ERROR;

        HashNode *destroy = entry_prev->next;
        entry_prev->next = destroy->next;

        PeerData *peer_prev = NULL;
        PeerData *peer_curr = curr->peerHead->next;
        for (int j = 0; j < curr->peerHead->max_size; j++) {
                peer_prev = peer_curr;
                if (peer_curr->next != NULL) {
                        peer_curr = peer_curr->next;
                }
                if (prev != NULL) {
                        free(peer_prev);
                }
        }
        free(destroy->peerHead);

        free(destroy);
        head->curr_count--;
        return OK;
}

// TODO : add add_filedata function etc
// befor using this function, PeerData should be initialized already
int add_peerData(HashHead *head, PeerData *data, char *hashkey) {
        HashNode *ret = NULL;
        ret_node = find_hashNode_by_key(head, hashkey);
        
        HashNode *new = NULL;

        if (ret_node == NULL) {
                // ini new HashNode
                new = malloc(sizeof(HashNode));
                strncpy(new->hashkey, hashkey, HASH_LEN);
                new->next = NULL;
                new->peerHead = malloc(PeerHead);
                new->peerHead->count = 0;
                new->peerHead->max_size = MAX_PEER_NUM_PER_FILE;
                peerHead->next = data;
                new->peerHead->count++;
                // add new HashNode
                add_hashNode(head, new);
        } else {
                // add PeerData
                if (ret_node->peerHead->count >= ret_node->peerHead->max_size) {
                        printf("warning : peerdatas reach max\n");
                }
                PeerData *peer_next = NULL;
                if (ret_node->peerHead->next != NULL) {
                        peer_next = ret_node->peerHead->next;
                }
                ret_node->peerHead->next = data;
                data->next = peer_next;
                ret_node->peerHead->max_size++;
        }
}

void ini_libsodium() {
        if (sodium_init() < 0) {
                fprintf(stderr, "libsodium ini fail\n");
                return EXIT_FAILURE;
        }
}

// calculate file's hashkey
int file_blake2b(const char *filename, char *hashkey) {  // output's len shoud be 65
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
#ifndef MSG_H
#define MSG_H

#define IP "192.168.1.100"
#define PORT 8888       
#define BACKLOG 32

#define OK 0
#define ERROR -1

#define S 64
#define M 256
#define L 1024

typedef struct DstData {
        int dst_sockfd;
        int dst_port;
        int self_sockfd;
        int self_port;
        
        int block_startpos;
        int block_size;
} DstData;

typedef struct Block {
        DstData dstData;
        _Bool ok;
        char data[L]; 
} Block;

typedef struct PeerData {
        int self_sockfd;
        int self_port;
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

typedef enum {
        REGISTER,       
        UNREGISTER,
        PEER_LIST,
        REQUEST_DOWNLOAD,
        RESPONSE_DOWNLOAD,
        REQUEST_UPLOAD,
        RESPONSE_UPLOAD
} MsgType;

typedef struct {
        MsgType msgtype;
        char *hashkey;
        PeerData *peerData;
        Block block;
        DstData dstData;
} MSG;

int recv_handler(int connect_fd, MSG *msg_send) {
        int nbytes;
        nbytes = recv(connect_fd, msg_send, sizeof(buf), 0);
        if (nbytes == -1) {
                perror("recv fail");
                return ERROR;
        } else if (nbytes == 0) {
                printf("connect closed\n");
                return OK;
        }
        return OK;
}

int send_handler(int connect_fd, MSG *recv_msg) {
        nbytes = send(connect_fd, recv_msg, sizeof(buf), 0);
        if (nbytes == -1) {
                perror("send fail");
                return ERROR;
        } else if (nbytes == 0) {
                printf("connect closed\n");
                return OK;
        }
        return OK;
}

#endif

#ifndef MSG_H
#define MSG_H

#define IP "192.168.1.100"
#define PORT 8888       
#define BACKLOG 32
#define MAX_FILE_SIZE 4096
#define MAX_PEER_CONNECTION 128

#define HASH_LEN 65
#define NAME_LEN 64
#define PATH_LEN 256

#define OK 0
#define ERROR -1

#define S 64
#define M 256
#define L 1024

// typedef struct PeerData {
//         int self_sockfd;
//         int self_port;
//         char filename[S];
//         char filepath[M];
//         long total_file_size;
//         PeerData *next;
// } PeerData;

// typedef struct PeerHead {
//         PeerData *next;
//         int count;
// } PeerHead;

// typedef struct HashNode {
//         PeerHead *peerHead;
//         char hashkey[HASH_LEN];
//         long total_file_size;
//         HashNode *next;
// } HashNode;

// typedef struct {
//         HashNode *next;
//         int count;
// } HashHead;

// typedef enum {
//         REGISTER,       
//         UNREGISTER,
//         PEER_LIST,
//         REQUEST_DOWNLOAD,
//         RESPONSE_DOWNLOAD,
//         REQUEST_UPLOAD,
//         RESPONSE_UPLOAD
// } MsgType;

// typedef struct Block_DstData {
//         int dst_sockfd;
//         int dst_port;
//         int self_sockfd;
//         int self_port;
        
//         long peer_block_startsize;
//         int peer_block_size;
//         long peer_block_endsize;

//         long block_startsize;
//         long block_total_size;

//         int puzzle_id;
//         int peers_num;
// } Block_DstData;

// typedef struct Block {
//         Block_DstData dstData;  // upload, download use
//         _Bool ok;
//         char data[L]; 
// } Block;

// typedef struct {
//         MsgType msgtype;
//         char *hashkey;
//         // register,query use
//         PeerData *peerData;     
//         // upload/download use
//         Block block;            
//         Block_DstData dstData;  // query peers use
// } MSG;

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

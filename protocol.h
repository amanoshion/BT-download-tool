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

typedef enum {
        REGISTER,
        UNREGISTER,
        PEER_LIST,
        REQUEST_PEER,
        RESPONSE_PEER,
} MsgType;

typedef struct {
        MsgType msgtype;
        char *hashkey;
        PeerData *peerData;
} MSG;

int recv_handler(int connect_fd, MSG *send_msg) {
        int nbytes;
        nbytes = recv(connect_fd, send_msg, sizeof(buf), 0);
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

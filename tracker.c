#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "protocol.h"
#include "tracker.h"

int request_dowload_func(HashHead *head, MSG *msg_recv, MSG *msg_send) {
        HashNode *ret_node = find_hashNode_by_key(head, msg->hashkey);
        if (ret_node == NULL) return ERROR;

        if (ret_node->peerHead->next != NULL) {
                PeerData *peer_curr = ret_node->peerHead->next;
        } else {
                return ERROR;
        }
        
        while(peer_curr != NULL) {
                memset(msg_send, 0, sizeof(MSG));
                msg_send->msgtype = RESPONSE_DOWNLOAD;
                msg_send->hashkey = msg_recv->hashkey;
                msg_send->peerData->dst_sockfd = msg_recv->peerData->self_sock;
                msg_send->peerData->dst_port = msg_recv->peerData->self_port;
                
                send_handler(msg_send);
        }

}

// TODO : fullfill this
void tracker_msg_handler(HashHead *head, MSG *msg) {
        switch(msg->msgtype) {
                case REGISTER:
                        add_peerData(head, msg->hashkey, msg->peerData);
                        break;
                case UNREGISTER:
                        // TODO : if peer close, active this func
                        if () {
                                unregister(head, hashkey, sockfd);
                        }
                        break;
                case REQUEST_DOWNLOAD:
                        // TODO : send msg to all sockfd under a hashnode, declare which file's block they should upload, and let them do RESPONSE_DOWNLOAD operation
                        
                        break;
                default:
                        break;
        }
}

int main(int argc, const char *argv[]) {
        ini_libsodium();

        // create socket stream
        int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1) {
                perror("create socket fail");
                return ERROR;
        }
        // define addr struct
        struct sockaddr_in trackerInfo;
        // clear addr struct
        memset(&trackerInfo, 0, sizeof(trackerInfo));
        // ini IPV4 family
        trackerInfo.sin_family = AF_INET;
        // ini socket port (endianness : host -> net)
        trackerInfo.sin_port = htons(PORT);
        // ini IP
        trackerInfo.sin_addr.s_addr = inet_addr(IP);
        // bind socket and address
        int bind_ret = bind(listen_fd, (struct sockaddr *)&trackerInfo, sizeof(trackerInfo));
        if (bind_ret == -1) {
                perror("bind fail");
                return ERROR;
        }

        int listen_ret = listen(listen_fd, BACKLOG);
        struct sockaddr_in clientInfo;
        memset(&clientInfo, 0, sizeof(clientInfo));
        socklen_t clientInfo_len = sizeof(clientInfo);

        // loop wait for client connecting . . .
        printf("tracker started\n");

        HashHead head = {0};
        ini_hash(&head);

        while(1) {
                int connect_fd = accept(listen_fd, (struct sockaddr *)&clientInfo, &clientInfo_len);
                if (connect_fd == -1) {
                        perror("connect fail");
                        return ERROR;
                }

                printf("client[%s : %d] connect to client\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));


                // handle send/recv
                while(1) {
                        MSG recv_msg = {0};
                        MSG send_msg = {0};

                        recv_handler(connect_fd, &recv_msg);
                        tracker_msg_handler(&recv_msg);

                }
        }

        free_hashTable(&head);
        return OK;
}

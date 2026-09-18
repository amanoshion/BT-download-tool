#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <math.h>
#include "protocol.h"
#include "tracker.h"

// handle peerlist operation

int handle_request_peerlist(HashHead *head, MSG *msg_recv) {
        HashNode *ret_node = find_hashNode_by_key(head, msg_recv->hashkey);
        if (ret_node == NULL) return ERROR;
        if (ret_node->peerHead->next != NULL) {
                PeerData *peer_curr = ret_node->peerHead->next;
        } else {
                return ERROR;
        }
        int peers_num = ret_node->peerHead->count;

        PeerData *curr_node = ret_node->peerHead->next;
        char buffer[L];
        MSG msg_send = {0};
        while(ret_node->peerHead->next != NULL) {
                memeset(buffer, 0, L);
                memset(&msg, 0, sizeof(MSG));
                snprintf(buffer, L, "sodkfd : %d\t port : %d\nfilename : %s\nhashkey : %s\n", 
                        curr_node->peerData->self_sock,
                        curr_node->peerData->self_port,
                        curr_node->filename,
                        curr_node->hashkey;
                );
                strncpy(msg_send.block.data, buffer, L);
                msg_send.block.dstData.dst_sockfd = msg_recv->dstData.self_sockfd;
                msg_send.block.dstData.dst_port = msg_recv->dstData.self_port;
                msg_send.block.dstData.peers_num = peers_num;
                send_handler(msg_recv->dstData.self_sockfd, msg_send);
        }
        return OK;
}

// handle request download
void split_block_for_peer(long *block_startsize, long *block_total_size, int peers_num) {
        static int block_sort_id = peers_num;
        long task_size = block_total_size - block_startsize;
        long peer_task_size = floor(task_size / peers_num);
        long peer_task_tail_size = task_size - peer_task_size*peers_num;
        for (int i = 0; i < peers_num - 1; i++) {
                // TODO
                *block_startsize = block_sort_id*peer_task_size + block
        }
}
int handle_request_download(HashHead *head, MSG *msg_recv) {
        HashNode *ret_node = find_hashNode_by_key(head, msg_recv->hashkey);
        if (ret_node == NULL) return ERROR;
        if (ret_node->peerHead->next != NULL) {
                PeerData *peer_curr = ret_node->peerHead->next;
        } else {
                return ERROR;
        }      

        // send msg to each peer
        MSG msg_send = {0};

        long block_startsize = msg_recv.dstData.block_startsize;
        long block_total_size = msg_recv->dstData.block_total_size;
        int peers_num = ret_node->peerHead->count;
        while(peer_curr != NULL) {
                memset(msg_send, 0, sizeof(MSG));

                msg_send.msgtype = REQUEST_UPLOAD;
                msg_send.hashkey = msg_recv->hashkey;
                msg_send.dstData.dst_sockfd = msg_recv->peerData->self_sock;
                msg_send.dstData.dst_port = msg_recv->peerData->self_port;
                if (msg_recv->dstData.block_total_size == 0) {
                        // calc file block_total_size
                        msg_recv->dstData.block_total_size = ret_node->total_file_size;
                }
                msg_send.dstData.block_startsize = msg_recv->dstData.block_startsize;
                msg_send.dstData.block_total_size = msg_recv->peerData->total_file_size;

                msg_send.dstData.curr_size = curr_size;

                send_handler(msg_send);
                peer_curr = peer_curr->next;
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
                        MSG *msg_send = {0};
                        request_dowload_func(head, msg, msg_send);
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
                        MSG msg_send = {0};

                        recv_handler(connect_fd, &recv_msg);
                        tracker_msg_handler(&recv_msg);

                }
        }

        free_hashTable(&head);
        return OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "protocol.h"
#include "peer.h"
#define DIR_PATH "./BT"

// register
int ensure_directory(const char *path) {
        struct stat st;

        if (stat(path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                        return OK;
                }
                return ERROR;
        }
        if (mkdir(path, 0755) == -1) {
                return ERROR;
        } 
        return OK;
}

void register_func(int connect_fd, MSG *msg, const char *path) {
        if ((ensure_directory(path)) == ERROR) return;

        DIR *dir = opendir(path);
        if (dir == NULL) {
                return;
        }
        struct dirent *entry;
        while((entry = readdir(path)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;                
                }
                memset(msg, 0, sizeof(MSG));
                msg->peerData->sockfd = connect_fd;
                msg->peerData->port = PORT;
                strcmp(msg->filename, entry->d_name);
                strcmp(msg->filepath, DIR_PATH);
                send_handler(msg);
        }
        return;
}
// request_peerlist
void request_peerlist(int connect_fd, char *hashkey) {
        MSG msg = {0};
        msg.msgtype = PEER_LIST;
        msg.hashkey = hashkey;
        send_handler(connect_fd, &msg);
}
// TODO : request download
// calc block_startpos and request to download a file by hashkey and send it 
void request_download_func(int connect_fd, char *hashkey, char *downloading_filename) {
        FILE *fd = fopen(downloading_filename, "r+");
        if (fd == NULL) {
                return;
        }
        
        // ini start pos
        MSG msg_send = {0};
        msg_send->msgtype = REQUEST_DOWNLOAD;
        strcmp(msg->hashkey, hashkey);
        msg_send.dstData.self_sockfd = connect_fd;
        msg_send.dstData.self_port = PORT;
        send_handler(msg_send);
}

// TODO : response_upload
DstData ini_dstData(MSG *msg_recv) {
        // ini dstData's information
        DstData dstData = {0};
        dstData.dst_sockfd = msg_recv->dstData.dst_sockfd;
        dstData.dst_port = msg_recv->dstData.dst_port;
        
        return 
}

// this func's block_size should calc the situation that file is at end
void prepare_file_block(DstData *dstData, char *file_path) {

        FILE *fd = fopen(file_path, "r");
        if (fd == NULL) {
                return block;
        } 

        // prepare block
        block.data = malloc(block_size * sizeof(char));

        int fwrite_ret = fwrite(block.data, 1, block_size, fd);
        if (fwrite_ret == block_size) {
                block.ok = 0;
        }

        free(block.data);
        fclose(fd);

        return;
}

int main(int argc, const char *argv[]) {
        // create socket stream
        int connect_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (connect_fd == -1) {
                perror("create socket fail");
                return ERROR;
        }

        // ini tracker information
        struct sockaddr_in trackerInfo = {0};
        // set IPV4 family
        trackerInfo.sin_family = AF_INET;
        // set port : host -> net
        trackerInfo.sin_port = htons(PORT);
        // set IP
        trackerInfo.sin_addr.s_addr = inet_addr(IP);

        // connect to tracker
        int ret = connect(connect_fd, (const struct sockaddr *)&trackerInfo, sizeof(trackerInfo));
        if (ret == -1) {
                perror("connect to tracker fail");
                return ERROR;
        }

        //register peers
        MSG msg = {0};
        register_func(connect_fd, msg, DIR_PATH);

        pid_t pid;
        pid = fork();
        if (pid < 0) {
                perror("fork fail");
                exit(EXIT_FAILURE);
        } else if (pid == 0) {  // son process, send msg
                MSG msg_send = {0};
                while(1) {
                        msg_send = {0};
                        // TODO : send options
                } 
                exit(EXIT_SUCCESS);
        } else {                // dad process, recv msg
                // TODO : create multi thread for download
                MSG recv_msg = {0};
                recv_handler(connect_fd, &recv_msg);

                // recycle son proccess
                int status;
                pid_t ret = waipid(pid, &status, 0);

                if (ret == -1) {
                        perror("dad waitpid fail");
                        exit(EXIT_FAILURE);
                }

                printf("dad exit\n");
        }

        return 0;
}

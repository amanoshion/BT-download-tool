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

#include <math.h>

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

void register_func(int connect_fd, const char *path) {
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
                MSG msg = {0};
                memset(msg, 0, sizeof(MSG));
                
                msg.peerData->sockfd = connect_fd;
                msg.peerData->port = PORT;

                fseek(fd, 0, SEEK_SET);
                msg.peerData->total_file_size = ftell(fd);
                rewind(fd);

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
// handle return of 'request peerlist' func
int ret_request_peerlist(MSG *msg_recv) {
        char buffer[L] = {0};
        fread(buffer, 1, L, msg_recv->block.data);
        printf("%s\n", buffer);
        return msg_recv->block.dstData.peers_num;
}

// TODO : request download
// calc block_startpos and request to download a file by hashkey and send it 

void request_download_func(int connect_fd, char *hashkey, char *downloading_filename) {
        static Download_Status status = idle;
        static Puzzle **puzzles = NULL;
        
        while(status != downloaded) {
                // get current peers num
                request_peerlist(connect_fd, hashkey);
                MSG msg_recv = {0};
                int peers_num = ret_request_peerlist(&msg_recv);
                MSG msg_send = {0};

                Puzzle temp_search_puzzle = {0};
                if (status == idle) {
                        puzzles = malloc(peers_num * sizeof(Puzzle));
                        // ini puzzles
                        int average_block = floor(msg_recv.dstData.block_total_size / peers_num);
                        for (int i = 0; i < peers_num - 1; i++) {
                                puzzles[i]->isok = NOTOK;
                                puzzles[i]->peer_block_startsize = i * average_block;
                                puzzles[i]->peer_block_size = average_block;
                                puzzles[i]->peer_block_endsize = puzzles[i]->peer_block_startsize + puzzles[i]->peer_block_size;
                        }
                        puzzles[peers_num - 1]->isok = NOTOK;
                        puzzles[peers_num - 1]->peer_block_startsize = (peers_num - 1) * average_block;
                        puzzles[peers_num - 1]->peer_block_endsize = msg_recv.dstData.block_total_size;
                        puzzles[peers_num - 1]->peer_block_size = puzzles[peers_num - 1]->peer_block_endsize - puzzles[peers_num - 1]->peer_block_startsize;

                        msg_send.dstData.block_startsize = 0;
                        msg_send.dstData.peer_block_size = msg_recv.dstData.block_total_size;
                        msg_send.dstData.peer_block_endsize =  msg_recv.dstData.block_total_size;
                        status = downloading;
                } else if (status == downloading) {
                        // TODO : pack below to a search func
                        int average_block = floor(msg_recv.dstData.block_total_size / peers_num);
                        temp_search_puzzle = {0};
                        for (int i = 0; i < peers_num; i++) {
                                if (puzzles[i]->isok == NOTOK) {
                                        _Bool found_next_finished_puzzles = 0;
                                        for(int j = i; j < peers_num; j++) {
                                                if (puzzles[j]->isok == OK) {
                                                        temp_search_puzzle.peer_block_startsize = puzzles[i]->peer_block_startsize;
                                                        temp_search_puzzle.peer_block_endsize = puzzles[j]->peer_block_endsize;
                                                        temp_search_puzzle.peer_block_size = temp_search_puzzle.peer_block_endsize - temp_search_puzzle.peer_block_startsize;
                                                        found_next_finished_puzzles = 1;
                                                } else {
                                                        continue;
                                                }

                                        }
                                        if (found_next_finished_puzzles == 0) {
                                                temp_search_puzzle.peer_block_endsize = puzzles[i]->peer_block_startsize;
                                                temp_search_puzzle.peer_block_endsize = puzzles[peers_num - 1]->peer_block_endsize;
                                                temp_search_puzzle.peer_block_size = temp_search_puzzle.peer_block_endsize - temp_search_puzzle.peer_block_startsize;
                                        }
                                } else if (puzzles[i]->isok == OK) {
                                        continue;
                                }
                        
                        }
                }
                msg_send->msgtype = REQUEST_DOWNLOAD;
                strcmp(msg->hashkey, hashkey);
                msg_send.dstData.self_sockfd = connect_fd;
                msg_send.dstData.self_port = PORT;
                msg_send.dstData.block_total_size = msg_recv.dstData.block_total_size;
                msg_send.dstData.block_startsize = msg_recv.dstData.block_startsize;

                msg_send.dstData.peer_block_startsize = temp_search_puzzle.peer_block_startsize;
                msg_send.dstData.peer_block_size = temp_search_puzzle.peer_block_size;
                msg_send.dstData.peer_block_endsize = temp_search_puzzle.peer_block_endsize;
                
                send_handler(msg_send);
        }
        free(puzzles);
        status = idle;
}

// this is the download peer's recieve func
// handle return of 'request upload' func : response_upload , it will makesure the peer loop download full file and finish request
void response_upload_func() {
        // TODO : update puzzles, check if download is ok
}

// TODO : response_download
// this is the upload peer's upload func
void response_download() {

}
void ini_dstData(MSG *msg_recv) {
        MSG msg_send = {0};
        // ini msg_send's information
        msg_send->msgtype = RESPONSE_UPLOAD;
        memset(msg_send.block, msg_recv->block, sizeof(Block));
        memset(msg_send.dstData, msg_recv->dstData, sizeof(Block_DstData));
        

        return 
}

// this func's block_size should calc the situation that file is at end
void prepare_file_block(Block_DstData *dstData, char *file_path) {

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

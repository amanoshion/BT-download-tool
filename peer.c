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

#include <sys/stat.h>
#include <fcntl.h>

#include "protocol.h"
#include "peer.h"
#include "thread.h"
#define DIR_PATH "./BT"

static thread_pool_t g_pool;
static int g_epfd = -1;

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

// calc block_startpos and request to download a file by hashkey and send it 
static Download_Status status = idle;
static Puzzle **puzzles = NULL;
void request_download_func(int connect_fd, char *hashkey) {
        
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
                                puzzles[i]->puzzel_id = i;
                        }
                        puzzles[peers_num - 1]->isok = NOTOK;
                        puzzles[peers_num - 1]->peer_block_startsize = (peers_num - 1) * average_block;
                        puzzles[peers_num - 1]->peer_block_endsize = msg_recv.dstData.block_total_size;
                        puzzles[peers_num - 1]->peer_block_size = puzzles[peers_num - 1]->peer_block_endsize - puzzles[peers_num - 1]->peer_block_startsize;
                        puzzles[i]->puzzel_id = peers_num -1;

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
                msg_send.block.dstData.self_sockfd = connect_fd;
                msg_send.block.dstData.self_port = PORT;
                msg_send.block.dstData.block_total_size = msg_recv.dstData.block_total_size;
                msg_send.block.dstData.block_startsize = msg_recv.dstData.block_startsize;

                msg_send.block.dstData.peer_block_startsize = temp_search_puzzle.peer_block_startsize;
                msg_send.block.dstData.peer_block_size = temp_search_puzzle.peer_block_size;
                msg_send.block.dstData.peer_block_endsize = temp_search_puzzle.peer_block_endsize;
                msg_send.block.dstData.puzzle_id = temp_search_puzzle.puzzel_id;
                
                send_handler(msg_send);
        }
        free(puzzles);
        status = idle;
}

// this is the download peer's recieve func
// handle return of 'request upload' func : response_upload , it will makesure the peer loop download full file and finish request
int response_upload_func(MSG *msg_recv, char *downloading_filename) {
        // TODO : update puzzles, check if download is ok
        char full_path[M];
        snprintf(full_path, "%s/%s", DIR_PATH, downloading_filename);
        _Bool isfinished = -1;
        static int current_file_size = 0;
        while(1) {
                if (msg->block.ok == NOTOK) {
                        int fd = open(full_path, WRONLY);
                        if (fd == -1) return ERROR;
                        write(fd, msg_recv->block.dstData.block_startsize, msg_recv->block.dstData.peer_block_size);
                        current_file_size += msg_recv->block.dstData.peer_block_size;
                        puzzles[msg_recv->block.dstData.puzzle_id]->isok = OK;
                        if (current_file_size == msg_recv->block.dstData.block_total_size) {
                                isfinished = 0;
                                break;
                        }
                } else {
                        continue;
                }
        }
        close(fd);
        if (isfinished == 0) {
                return OK;
        }
        return ERROR;
}

// response_download
// this is the upload peer's upload func
void response_download(MSG *msg_recv, char *hashkey) {
        if ((ensure_directory(DIR_PATH)) == ERROR) return;

        DIR *dir = opendir(DIR_PATH);
        if (dir == NULL) {
                return;
        }

        MSG msg = {0};
        struct dirent *entry;
        while((entry = readdir(path)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;                
                }
                memset(msg, 0, sizeof(MSG));
                int fd = open(entry->d_name, "r");
                // TODO : finish this
                lseek(fd, msg_recv->dstData.block_startsize, SEEK_SET);
                read(fd, msg.block.data, msg_recv->block.dstData.peer_block_size);
                lseek(fd, 0, SEEK_SET);
        }
        send_handler(msg);
        return;
}

void ini_dstData(MSG *msg_recv) {
        MSG msg_send = {0};
        // ini msg_send's information
        msg_send->msgtype = RESPONSE_UPLOAD;
        memset(msg_send.block, msg_recv->block, sizeof(Block));
        memset(msg_send.dstData, msg_recv->dstData, sizeof(Block_DstData));
        return;
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

        /* ep. ini */
        int epfd;
        struct epoll_event ev, events[MAX_EVENTS];

        // connect to tracker
        int ret = connect(connect_fd, (const struct sockaddr *)&trackerInfo, sizeof(trackerInfo));
        if (ret < 0 && erno != EINPROGRESS) {
                perror("connect to tracker fail");
                exit(EXIT_FAILURE);
        }
        /* ep. create epoll*/
        epfd = epoll_create1(0);
        if (epfd < 0) {
                perror("epoll_create1");
                exit(1);
        }
        g_epfd = epfd;

        ev.events       = EPOLLOUT | EPOLLET | EPOLLONESHOT;
        ev.data.fd      = sockfd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
                perror("epoll_ctl ADD");
                exit(1);
        }

        if (pool_init(&g_pool, THREAD_NUM, QUEUE_SIZE) < 0) {
                fprintf(stderr, "thread pool ini fail");
                exit(1);
        }
        int connected = 0;
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

                /* ep. wait ready epoll */
                while(1) {
                        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
                        if (n < 0) {
                                if (errno == EINTR) continue;
                                perror("epoll_wait");
                                break;
                        }
                        
                        for (int i = 0; i < n; i++) {
                                int fd = evnets[i].data.fd;
                                uint32_t revents = events[i].events;
                                if (!connected && (revents & EPOLLOUT)) {
                                        int error = 0;
                                        socklen_t len = sizeof(error);
                                        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                                                fprintf(stderr, "connect fail : %s\n", strerror(error));
                                                close(fd);
                                                g_sockfd = -1;
                                                goto cleanup;
                                        }
                                        printf("connect success! fd = %d\n", fd);
                                        connected = 1;

                                        epoll_mod(epfd, fd, EPOLLIN);
                                        if (connected && (revents & EPOLLIN)) {
                                                pool_add_task(&g_pool, fd, revents);
                                        }
                                        if (revents & (EPOLLERR | EPOLLHUP)) {
                                                printf("error happen or tracker close\n");
                                                close(fd);
                                                g_sockfd = -1;
                                                goto cleanup;
                                        }
                        }               

                }

                // recycle son
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

cleanup:
    pool_destroy(&g_pool);
    if (g_sockfd != -1) close(g_sockfd);
    close(epfd);
    printf("client quit\n");
    return 0;
}

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

#include "protocol.h"
// TODO :
void peer_msg_handler(MSG *msg) {
        switch(msg->msgtype) {
                case REQUEST_PEER:
                        break;
                case RESPONSE_PEER:
                        break;
        }
        return;
}

void peer_msg_input(MSG *msg) {

}
int main(int argc, const char *argv[]) {
        // create socket stream
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_fd == -1) {
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
        int ret = connect(socket_fd, (const struct sockaddr *)&trackerInfo, sizeof(trackerInfo));
        if (ret == -1) {
                perror("connect to tracker fail");
                return ERROR;
        }

        // TODO : register peers, add files into peerData and send to tracker
        while(1) {
                MSG send_msg = {0};
                send_handler(connect_fd, &send_msg);
        }

        pid_t pid;
        pid = fork();
        if (pid < 0) {
                perror("fork fail");
                exit(EXIT_FAILURE);
        } else if (pid == 0) {  // son process, send msg
                MSG send_msg = {0};
                while(1) {
                        send_msg = {0};
                        // TODO : fullfill msg
                        send_handler(connect_fd, &send_msg);
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

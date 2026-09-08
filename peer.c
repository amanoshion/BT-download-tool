#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "protocol.h"

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

        char buf[L] = {0};
        while(true) {
                MSG recv_msg = {0};
                MSG send_msg = {0};

                recv_handler(connect_fd, &recv_msg);
                
                send_handler(connect_fd, &send_msg);
        }        
}

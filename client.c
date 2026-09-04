#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define OK 0
#define ERROR -1

#define IP "192.168.1.100"
#define PORT 8888

#define S 64
#define M 256
#define L 1024

int main(int argc, const char *argv[]) {
        // create socket stream
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_fd == -1) {
                perror("create socket fail");
                return ERROR;
        }

        // ini server information
        struct sockaddr_in serverInfo = {0};
        // set IPV4 family
        serverInfo.sin_family = AF_INET;
        // set port : host -> net
        serverInfo.sin_port = htons(PORT);
        // set IP
        serverInfo.sin_addr.s_addr = inet_addr(IP);

        // connect to server
        int ret = connect(socket_fd, (const struct sockaddr *)&serverInfo, sizeof(serverInfo));
        if (ret == -1) {
                perror("connect to server fail");
                return ERROR;
        }

        char buf[L] = {0};
        while(true) {
                memset(buf, 0, sizeof(buf), 0);
                // send message
                int nbytes = send(socket_fd, buf, sizeof(buf), 0);
                if (nbytes == -1) {
                        perror("send fail");
                        return ERROR;
                }
                // recv message
                memset(buf, 0, sizeof(buf));
                int nbytes_recv = recv(socket_fd, buf, sizeof(buf), 0);
                if (nbytes_recv == -1) {
                        perror("recv fail");
                } else if (nbytes_recv == 0) {
                        printf("server close connect\n");
                        return 0;
                }
                printf("message from server : %s\n", buf);
                return OK;
        }        
}
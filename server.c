#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet>
#include <unistd.h>

#define IP "192.168.1.100"
#define PORT 8888

#define OK 0
#define ERROR -1

#define S 64
#define M 256
#define L 1024
int main(int argc, const char *argv[]) {
        // create socket stream
        int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1) {
                perror("create socket fail");
                return ERROR;
        }
        // define addr struct
        struct sockaddr_in serverInfo;
        // clear addr struct
        memset(&serverInfo, 0, sizeof(serverInfo));
        // ini IPV4 family
        serverInfo.sin_family = AF_INET;
        // ini socket port (endianness : host -> net)
        serverInfo.sin_port = htons(PORT);
        // ini IP
        serverInfo.sin_addr.s_addr = inet_addr(IP);
        // bind socket and address
        int ret = bind(listen_fd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
        if (ret == -1) {
                perror("bind fail");
                return ERROR;
        }
        struct sockaddr_in clientInfo;
        memset(&clientInfo, 0, sizeof(clientInfo));
        socklen_t clientInfo_len = sizeof(clientInfo);

        // loop wait for client connecting . . .
        printf("server started\n");
        while(1) {
                int connect_fd = accept(listen_fd, (struct sockaddr *)&clientInfo, &clientInfo_len);
                if (connect_fd == -1) {
                        perror("connect fail");
                        return ERROR;
                }

                printf("client[%s : %d] connect to server\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));
                // handle networking
                while(1) {
                        char buf[L] = {0};
                        int nbytes = recv(connect_fd, buf, sizeof(buf), 0);
                        if (nbytes == -1) {
                                perror("recv fail");
                                return ERROR;
                        }
                        
                }
        }
        return OK;
}

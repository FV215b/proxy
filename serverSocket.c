#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
//#include "proxy.h"
#define PORT 6666
#define BUFF_SIZE 4096
#define RESP "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-type: text/html\r\nContent-length: 100\r\n\r\n<html>\r\n<body>\r\n\t<h1>Hello, World!</h1>\r\n</body>\r\n</html>\r\n"

int main(int argc, char const *argv[])
{
    struct sockaddr_in server;
    unsigned int socket_len = sizeof(server);
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM | !O_NONBLOCK, 0);
    if(socket_fd < 0){
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, socket_len);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind */
    int bind_status = bind(socket_fd, (struct sockaddr *)&server, socket_len);
    if(bind_status < 0){
        perror("Bind error\n");
        exit(EXIT_FAILURE);
    }

    /* Listen */
    int listen_status = listen(socket_fd, 10);
    if(listen_status < 0){
        perror("Listen error\n");
        exit(EXIT_FAILURE);
    }

    /* Accept */
    while(1){
        int conn_fd = accept(socket_fd, (struct sockaddr *)&server, &socket_len);
        if(conn_fd < 0){
            perror("Connection error\n");
            exit(EXIT_FAILURE);
        }
        printf("Build connection\n");
        char buff[BUFF_SIZE];
        memset(buff, 0, BUFF_SIZE);
        int receive_len = recv(conn_fd, buff, BUFF_SIZE, 0);
        if(receive_len < 0){
            perror("Failed to receive message\n");
            printf("%d: %s\n", errno, strerror(errno));
            //exit(EXIT_FAILURE);
        }
        else if(receive_len == 0){
            printf("No message received\n");
        }
        else{
            printf("%s\n", buff);
        }
        //req_sock *newsock;
        //char *newbuff = parsingRequest(buff, newsock);
        char *newbuff = RESP;
        int send_status = send(conn_fd, newbuff, strlen(newbuff)-1, 0);
        if(send_status < 0){
            perror("Failed to send response\n");
            exit(EXIT_FAILURE);
        }
        printf("Response is successfully sent\n");
        close(conn_fd);
        printf("Connection closed\n");
    }

    return EXIT_SUCCESS;
}




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define BUFF_SIZE 1024

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
    server.sin_port = htons(6666);
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
            printf("Got the message\n");
            printf("MSG: %s\n", buff);
        }
        close(conn_fd);
        printf("Connection closed\n");
    }

    return EXIT_SUCCESS;
}




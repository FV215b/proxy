#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define BUFF_SIZE 1024
#define MSG "Hello world!"

int main(int argc, char const *argv[])
{
    struct sockaddr_in client;
    unsigned int socket_len = sizeof(client);
    if(argc != 2){
        perror("Input ip address\n");
        exit(EXIT_FAILURE);
    }
    
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&client, 0, socket_len);
    client.sin_family = AF_INET;
    client.sin_port = htons(6666);
    int ip_status = inet_pton(AF_INET, argv[1], &client.sin_addr);
    if(ip_status != 1){
        perror("Failed to convert argv[1] to ip address\n");
        exit(EXIT_FAILURE);
    }

    /* Connect */
    int conn_fd = connect(socket_fd, (struct sockaddr *)&client, socket_len);
    if(conn_fd < 0){
        perror("Connection error\n");
        exit(EXIT_FAILURE);
    }
    printf("Build connection\n");
    char buff[BUFF_SIZE];
    fgets(buff, BUFF_SIZE, stdin);
    int send_status = send(socket_fd, buff, strlen(buff)-1, 0);
    if(send_status < 0){
        perror("Failed to send message\n");
        exit(EXIT_FAILURE);
    }
    printf("Message is successfully sent\n");
    //printf("MSG: %s\n", buff);
    close(socket_fd);

    return EXIT_SUCCESS;
}
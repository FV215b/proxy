#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define BUFF_SIZE 104800
#define MSG "GET http://www.cplusplus.com/reference/unordered_map/unordered_map/ HTTP/1.1\r\nHost: www.cplusplus.com\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSGoogle "GET https://www.google.com/ HTTP/1.1\r\nProxy-Connection: Keep-Alive\r\n\r\n"
int main(int argc, char const *argv[])
{
    if(argc != 2){
        perror("Input port\n");
        exit(EXIT_FAILURE);
    }
    int po = atoi(argv[1]);
    if(po < 1024){
        perror("Engaging reserved port is banned\n");
        exit(EXIT_FAILURE);
    }
    char *buff = MSG;
    struct sockaddr_in client;
    unsigned int socket_len = sizeof(client);
    
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&client, 0, socket_len);
    client.sin_family = AF_INET;
    client.sin_port = htons(po);
    int ip_status = inet_pton(AF_INET, "127.0.0.1", &client.sin_addr);
    if(ip_status != 1){
        perror("Failed to construct socket\n");
        exit(EXIT_FAILURE);
    }

    /* Connect */
    int conn_fd = connect(socket_fd, (struct sockaddr *)&client, socket_len);
    if(conn_fd < 0){
        perror("Connection error\n");
        exit(EXIT_FAILURE);
    }
    printf("Build connection\n");
    int send_status = send(socket_fd, buff, strlen(buff)+1, 0);
    if(send_status < 0){
        perror("Failed to send message\n");
        exit(EXIT_FAILURE);
    }
    printf("Message is successfully sent\n");
    printf("MSG: %s\n", buff);
    char newbuff[BUFF_SIZE];
    memset(newbuff, 0, BUFF_SIZE);
    int receive_len = recv(socket_fd, newbuff, BUFF_SIZE, 0);
    if(receive_len < 0){
        perror("Failed to receive response\n");
        printf("%d: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if(receive_len == 0){
        printf("No response received\n");
    }
    else{
        printf("%s\n", newbuff);
    }
    close(socket_fd);

    return EXIT_SUCCESS;
}
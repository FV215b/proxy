#include "socket.h"

char* clientSock(char* host, char* port, char* buff){
    int socket_fd;
    struct addrinfo client; 
    struct addrinfo* clientlist;
    
    /* Create socket */
    memset(&client, 0, sizeof(client));
    client.ai_family = AF_INET;
    client.ai_socktype = SOCK_STREAM;
    int info_status = getaddrinfo(host, port, &client, &clientlist);
    if(info_status != 0){
        perror("Failed to get server information\n");
        freeaddrinfo(clientlist);
        return NULL;
    }

    /* Connect */
    struct addrinfo* s;
    for(s = clientlist; s != NULL; s = s->ai_next){
        if((socket_fd = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) < 0){
            perror("Try nexy connection option\n");
            continue;
        }
        if(connect(socket_fd, s->ai_addr, s->ai_addrlen) < 0){
            perror("Connection error\n");
            close(socket_fd);
            continue;
        }
        break;
    }
    if(s == NULL){
        fprintf(stderr, "failed to connect\n");
        freeaddrinfo(clientlist);
        return NULL;
    }
    printf("Build connection\n");
    int send_status = send(socket_fd, buff, strlen(buff), 0);
    if(send_status < 0){
        perror("Failed to send message\n");
        close(socket_fd);
        freeaddrinfo(clientlist);
        return NULL;
    }
    printf("Message is successfully sent\n");
    char newbuff[BUFF_SIZE];
    memset(newbuff, 0, BUFF_SIZE);
    int receive_len = recv(socket_fd, newbuff, BUFF_SIZE-1, 0);
    if(receive_len < 0){
        perror("Failed to receive response\n");
        printf("%d: %s\n", errno, strerror(errno));
        close(socket_fd);
        freeaddrinfo(clientlist);
        return NULL;
    }
    else if(receive_len == 0){
        printf("No response received\n");
        close(socket_fd);
        freeaddrinfo(clientlist);
        return NULL;
    }
    printf("%s\n", newbuff);
    close(socket_fd);
    freeaddrinfo(clientlist);
    char* ret = malloc(strlen(newbuff)+1);
    strcpy(ret, newbuff);
    return ret;
}

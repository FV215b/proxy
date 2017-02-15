#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "parser.h"
#include "cache.h"
#include "socket.h"

int main(int argc, char const *argv[])
{
    if(argc != 2){
        perror("Input listening port\n");
        exit(EXIT_FAILURE);
    }
    int po = atoi(argv[1]);
    if(po < 1024){
        perror("Engaging reserved port is banned\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server;
    unsigned int socket_len = sizeof(server);

    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server, 0, socket_len);
    server.sin_family = AF_INET;
    server.sin_port = htons(po);
    server.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind */
    int bind_status = bind(socket_fd, (struct sockaddr *)&server, socket_len);
    if(bind_status < 0){
        perror("Bind error\n");
        exit(EXIT_FAILURE);
    }

    /* Listen */
    int listen_status = listen(socket_fd, 5);
    if(listen_status < 0){
        perror("Listen error\n");
        exit(EXIT_FAILURE);
    }
    int daemon_status = daemon(0, 1);
    if(daemon_status != 0){
        perror("Daemon create failed\n");
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
        memset(buff, '\0', BUFF_SIZE);
        int receive_len = recv(conn_fd, buff, BUFF_SIZE, 0);
        if(receive_len < 0){
            perror("Failed to receive response\n");
            printf("%d: %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if(receive_len == 0){
            printf("No response received\n");
        }
        else{
            printf("Receive request:\n%s\n", buff);
        }

        req_info* reqinfo = (req_info *)malloc(sizeof(req_info));
        if(reqinfo == NULL){
            perror("Allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        char* sendbuff = parse_request(buff, reqinfo);
        printf("Length = %lu\nParsing result:\n%s", strlen(sendbuff), sendbuff);
        printf("Method: %s\nService: %s\nHost: %s\nPort: %d\nCompURL: %s\nPartURL: %s\n", reqinfo->method, reqinfo->prtc, reqinfo->host, reqinfo->port, reqinfo->c_url, reqinfo->p_url);
        char* cache_read = NULL;
        if(strcmp(reqinfo->method, "GET") == 0){
            cache_read = readCache(reqinfo->c_url);
        }
        int send_status;
        if(cache_read == NULL){
            char* newbuff = clientSock(reqinfo->host, reqinfo->prtc, sendbuff);
            rsp_info* rspinfo;
            rspinfo = parse_response(newbuff);
            if(strcmp(reqinfo->method, "GET") == 0 && rspinfo->code == 200){
                allocCache(newbuff, reqinfo->c_url, 3);
            }
            send_status = send(conn_fd, newbuff, strlen(newbuff)+1, 0);
            if(rspinfo->date != NULL){
                free(rspinfo->date);
            }
            if(rspinfo->expire != NULL){
                free(rspinfo->expire);
            }
            if(rspinfo->length != NULL){
                free(rspinfo->length);
            }
            if(rspinfo->cache != NULL){
                free(rspinfo->cache);
            }
            free(rspinfo->status);
            free(rspinfo);
            free(newbuff);
        }
        else{
            send_status = send(conn_fd, cache_read, strlen(cache_read)+1, 0);
        }
        if(send_status < 0){
            perror("Failed to send response\n");
            exit(EXIT_FAILURE);
        }
        printf("Response is successfully sent\n");
        close(conn_fd);
        free(reqinfo->host);
        free(reqinfo->c_url);
        free(reqinfo->p_url);
        free(reqinfo);
        
        printf("Connection closed\n");
    }

    return EXIT_SUCCESS;
}

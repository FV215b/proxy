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
//#define PORT 6666
//#define BUFF_SIZE 4096
#define RESP "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-type: text/html\r\nContent-length: 100\r\n\r\n<html>\r\n<body>\r\n<h1>Hello, World!</h1>\r\n</body>\r\n</html>\r\n"

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
    int socket_fd = socket(AF_INET, SOCK_STREAM | !O_NONBLOCK, 0);

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
        /* 
            Request parsing(buff)
                extract method GET/POST/CONNECT, only GET will search in cache
                extract completed URL to search cache as key
            
            Request parsing for socket(buff)
                extract hostname, port to create socket
                change request line from completed URL to partial URL
                add host if header doesn't have
                add connection & proxy-connection as "close" if header doesn't have

            Response parsing(resp)
                expire time(if non-exist, give default value)

            Response parsing for cache(resp)
                change date
            
            *********************************
            Request parsing
                extract method GET/POST/CONNECT, only GET will search in cache
                extract completed URL to search cache as key
            read cache
            if non-exist
                Request parsing for socket
                    extract hostname, port to create socket
                    change request line from completed URL to partial URL
                    add host if header doesn't have
                    add connection & proxy-connection as "close" if header doesn't have
                build up server socket
                wait for response
                Response parsing
                    expire time(if non-exist, give default value)
                memory allocation for response
            else
                Response parsing for cache
                    change date
            send back
            *********************************

            what todo:
                extra long request/response cut into multiple http packages
                CONNECT mode
                only cache GET
                support port listen configuation
        */
        req_info* reqinfo = (req_info *)malloc(sizeof(req_info));
        if(reqinfo == NULL){
            perror("Allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        char* sendbuff = parse_request(buff, reqinfo);
        printf("Length = %lu Parsing result:\n%s", strlen(sendbuff), sendbuff);
        /* If reqinfo->method == GET */
        char* cache_result = NULL;
        if(strcmp(reqinfo->method, "GET") == 0){
            cache_result = readCache(reqinfo->c_url);
        }
        char* newbuff;
        if(cache_result == NULL){
            printf("Method: %s\nService: %s\nHost: %s\nPort: %d\nCompURL: %s\nPartURL: %s\n", reqinfo->method, reqinfo->prtc, reqinfo->host, reqinfo->port, reqinfo->c_url, reqinfo->p_url);
            //newbuff = clientSock(reqinfo->host, reqinfo->prtc, sendbuff);
            newbuff = clientSock(reqinfo->host, reqinfo->prtc, sendbuff);
            if(newbuff == NULL){
                perror("Parsing failed\n");
                exit(EXIT_FAILURE);
            }
            /* If method == GET && status code == 200 */
            /* allocCache(newbuff, reqinfo->c_url, resinfo->time); */
            if(strcmp(reqinfo->method, "GET") == 0){
                allocCache(newbuff, reqinfo->c_url, EXPIRE_TIME);
            }
            printCache();
        }
        else{
            newbuff = cache_result;
            printCache();
        }
        int send_status = send(conn_fd, newbuff, strlen(newbuff)-1, 0);
        if(send_status < 0){
            perror("Failed to send response\n");
            exit(EXIT_FAILURE);
        }
        printf("Response is successfully sent\n");
        close(conn_fd);
        free(reqinfo);
        printf("Connection closed\n");
    }

    return EXIT_SUCCESS;
}

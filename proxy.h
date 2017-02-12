#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

/* required by server-side socket 
struct _req_sock{
	char *host;
	char *serv;
};
typedef struct _req_sock req_sock;*/

struct _cache{
	char* url;
	char* res;
	int ext;
	struct _cache* next;
	struct _cache* prev;
};
typedef struct _cache cache;

cache* head = NULL;
cache* tail = NULL;
int obj_num = 0;

//char* parsingRequest(char *buff, req_sock *sock);
bool allocCache(char* buff, char* url, int extime);
char* readCache(char* url);
bool scanCache(char* url);
void moveCache(cache* curr);
void deleteCache(cache* curr);
void printCache();

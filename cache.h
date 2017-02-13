#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define MAX_OBJECT_NUM 4
#define MAX_OBJECT_SIZE 1000
#define EXPIRE_TIME 10

struct _cache{
	char* url;
	char* res;
	int ext;
	struct _cache* next;
	struct _cache* prev;
};
typedef struct _cache cache;

//char* parsingRequest(char *buff, req_sock *sock);
bool allocCache(char* buff, char* url, int extime);
char* readCache(char* url);
bool scanCache(char* url);
void moveCache(cache* curr);
void deleteCache(cache* curr);
void printCache();

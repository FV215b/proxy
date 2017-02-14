#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

#define IS_SPACE(x) isspace((int)(x))

#define BUF_SIZE  8192
#define MAX_HEADER_SIZE  8192


#define SERVER_SOCKET_ERROR  -1
#define SERVER_SETSOCKOPT_ERROR -2
#define SERVER_BIND_ERROR -3
#define SERVER_LISTEN_ERROR -4
#define CLIENT_SOCKET_ERROR -5
#define CLIENT_RESOLVE_ERROR -6
#define CLIENT_CONNECT_ERROR -7
#define HEADER_BUFFER_FULL -8
#define BAD_HTTP_PROTOCOL -9


typedef struct req_info_t{
  char method[10];
  char prtc[10];
  char* host;
  int port;
  char* c_url;
  char* p_url;
}req_info;


typedef struct rsp_info_t{
  int code;
  char status[128];
  char server[128];
  char date[128];
  char expire[128];
  
}rsp_info;

void* memmem(const void*, size_t, const void*, size_t);
char* parse_request(char* buff, req_info* reqinfo);
//char* rewrite_request(req_info*);
rsp_info parse_response(char*);

#ifndef HAVE_MEMMEM

#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
#endif

/* Return the first occurrence of NEEDLE in HAYSTACK.  */
void *memmem (const void *haystack, size_t haystack_len, const void *needle,size_t needle_len){
  const char *begin;
  const char *const last_possible  = (const char *) haystack + haystack_len - needle_len;
  
  if (needle_len == 0)
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
    return (void *) haystack;
  
  /* Sanity check, otherwise the loop might search through the whole
     memory.  */
  if (__builtin_expect (haystack_len < needle_len, 0))
    return NULL;
  
  for (begin = (const char *) haystack; begin <= last_possible; ++begin)
    if (begin[0] == ((const char *) needle)[0] &&
	!memcmp ((const void *) &begin[1],
		 (const void *) ((const char *) needle + 1),
		 needle_len - 1))
      return (void *) begin;
  
  return NULL;
}
#endif

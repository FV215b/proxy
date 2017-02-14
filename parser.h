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
#include <time.h>

#define IS_SPACE(x) isspace((int)(x))


/* struct tm { */
/*   int tm_sec;    /\* Seconds (0-60) *\/ */
/*   int tm_min;    /\* Minutes (0-59) *\/ */
/*   int tm_hour;   /\* Hours (0-23) *\/ */
/*   int tm_mday;   /\* Day of the month (1-31) *\/ */
/*   int tm_mon;    /\* Month (0-11) *\/ */
/*   int tm_year;   /\* Year - 1900 *\/ */
/*   int tm_wday;   /\* Day of the week (0-6, Sunday = 0) *\/ */
/*   int tm_yday;   /\* Day in the year (0-365, 1 Jan = 0) *\/ */
/*   int tm_isdst;  /\* Daylight saving time *\/ */
/* }; */

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
  char cache[128];
  char* date;
  char* expire;
  char* Etag;
  char conncection[30];
}rsp_info;

void* memmem(const void*, size_t, const void*, size_t);
char* parse_request(char* buff, req_info* tokens);
//char* rewrite_request(req_info*);
rsp_info parse_response(char*);

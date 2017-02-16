#include "parser.h"
#include "cache.h"

#define notInCache 0  //cannot find in cache
#define cacheExpire 1 //in cache but expired
#define isValid 2 //in cache and not expired
//#define reqValid 4 // the preferred behavior for an HTTP/1.1 origin server is to send both a strong entity tag and a Last-Modified value.
#define reqReValid 5 //if revalidate, resend the Etag to compare if changed
#define notCacheable 4



int isExpired(char*, double);
int isCacheable(rsp_info*);
int cacheStatus(int);
int respStatus(rsp_info*);



#include "cache_policy.h"

#include <time.h>


void logRequest(FILE*, char*, int, char*);
void logCheckRequest(FILE*, int, char*, int);
void logServer(FILE*, char*, char*, char*, int);
void logOkCheck(FILE*, int,  char*);
void logRespClient(FILE*, char*, int);
void logTunnelOk(FILE*, int);
void logC_control(FILE*, char*);
void logEtag(FILE*, char*);

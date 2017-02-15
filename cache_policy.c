#include "cache_policy.h"

int isExpired(char* date, double count){
  if(date == NULL){
    printf("Failed to get the date of response in cache\n");
    return 0;
  }
  int exp = -1;
  double diff = curMinusDate(date) - count;
  
#ifdef DEBUG
    printf("new - cur in seconds is: %f\n", diff);
#endif
    if(diff >= 0){
#ifdef DEBUG
      printf("DEBUG:  expired\n");
#endif
      exp = 1;
    }else{
#ifdef DEBUG
      exp = 0;
#endif
    }
  return exp;
}

/*see if cacheable or need revalidation*/
int isCacheable(rsp_info* response){
  int is_cacheable = -1;
  if(strstr(response->cache,"no-cache")){ 
    is_cacheable = 0;
  }
  else if((strstr(response->cache, "max-age = 0"))){
    is_cacheable = 1;
  }
  return is_cacheable;
}

/*use weak(Last-Modified time value) and strong validators(entity tag) to check for conditions*/
/*need to compare these validatory with the latest one, is there any way to get the head of the cache?*/
/*need to check whether the head is avaliable, if null, response is the first one, must valid??*/

/*see if expired, in cache, valid, need validation*/
int cacheStatus(int status){
  switch(status){
  case 0:
    return 0; //not in cache
  case 1:
    return 1; //in cache but expired
  case 2:
    return 2;
  }
  return -1;
}
  //int status = -1;
 /*  if(request != NULL && scanCache(request->c_url)){ */
/*     char* match_resp = readCache(request->c_url); */
/* #ifdef DEBUG */
/*     printf("DEBUG: scanned: in cache, requires validation, match is:%s\n",match_resp); */
/* #endif */
/*     //    status = 1; */
/*     if(isExpired(match_resp,count)){ //what did the cache return? how to check for expiration if not a whole struct */
/* #ifdef DEBUG */
/*       printf("DEBUG: has expired\n"); */
/* #endif */
/*       return 4; */
/*     }else{ */
/* #ifdef DEBUG */
/*       printf("DEBUG: not expired\n"); */
/* #endif */
/*       if(checkValidator(match_resp,fresh)){ //TODO:check the one in cache(stale and fresh?) with its Last-Modified entity and Etag entitiy for validating */
/* #ifdef DEBUG */
/* 	printf("DEBUG: after checking, is valid\n"); */
/* #endif */
/* 	return 3; */
/*       }else{ */
/* #ifdef DEBUG */
/* 	printf("DEBUG: after checking, not valid, just not expired\n"); */
/* #endif */
/* 	return 1; */
/*       } */
/*     } */
/*   }else{ */
/*     return 0; */
/*   } */



#include "parser.h"

#define DEBUG

//char* buffer = "   CONNECT ftp://www.cmu.edu:8088/hub/index.html HTTP/1.1 lala \r\nthis is a bullshit header\r\nConnection:close\r\n\r\n";
char* buffer = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.google.com/\r\nContent-Type: text/html; charset=UTF-8\r\nDate: Sun, 26 Apr 2009 11:11:49 GMT\r\nExpires: Tue, 26 May 2009 11:11:49 GMT\r\nX-$PrototypeBI-Version: 1.6.0.3\r\nCache-Control: public, max-age=2592000\r\nServer: gws\r\nContent-Length:  219  \r\n\r\n";
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

  #ifdef DEBUG
  printf("START:\n");
  #endif
  rsp_info* tokens = (rsp_info*)malloc(sizeof(rsp_info));
  if(tokens == NULL){
    free(tokens);
    perror("Failed to malloc\n");
    return NULL;
  }
#ifdef DEBUG
  printf("malloced for tokens\n");
#endif
  const char* curPos = buffer;
  const char* endline = (const char*)memmem(curPos,strlen(buffer)-(curPos-buffer),"\r\n",2);
  if(endline == 0){
    printf("Request doesn't end with \\r\\n\n");
    return NULL;
  }
  size_t i = 0;
  //skip space
  while(IS_SPACE(buffer[i])){
    i++;
  }
  #ifdef DEBUG
  printf("skiped space\n");
  #endif
  if(i >= strlen(buffer)-1){
    printf("Too many spaces, out of buffer bound\n");
    return NULL;
  }
  char* m_s = buffer+i; //first non-space char, should be the start of the method
  if(m_s){
    #ifdef DEBUG
    printf("The input response line is:%s\n", m_s);
    #endif
    char* H_e = strstr(m_s,"HTTP/1.1"); //first space after the start, should be the end of the method
    if(H_e){
      char* c_s = strchr(H_e,' ');
      if(c_s){
	char* c_e = strchr(c_s + 1, ' ');
	if(c_e){
	  char s_code[10];  
	  strncpy(s_code, c_s, (int)(c_e - c_s));
#ifdef DEBUG
	  printf("DEBUG: status code is: %s\n", s_code);
#endif
	  tokens->code = atoi(s_code); //convert status code string into int
#ifdef DEBUG
	  printf("DEBUG: status code int is: %d\n", tokens->code);
#endif
	  while(IS_SPACE(*(c_e + 1))){
	    c_e++;
	  }
	  char* m_s = c_e + 1; //start of the status message
	  if(m_s){
	    char* m_e = strstr(m_s,"\r\n");
	    if(m_e){
	      strncpy(tokens->status, m_s, (int)(m_e - m_s));
#ifdef DEBUG
	      printf("DEBUG: status message is: %s\n", tokens->status);
#endif
	      while(IS_SPACE(*(m_e + 2))){
		m_e++;
	      }
	      char* h_s = m_e+2; //start of the headers
	      if(h_s){ //find the server and the date
		char* s_s = strstr(h_s, "Server: ");
		if(!s_s){
		  printf("No server header in the response\n");
		  strncpy(tokens->server,"Unknown",strlen("Unknown"));
		  
		}else{
		  char* s_e = strstr(s_s,"\r\n");
		  //printf("DEBUG:s_e is:%s\n",s_e);
		  strncpy(tokens->server, s_s+8,(int)(s_e - s_s)-8);
#ifdef DEBUG
		  printf("DEBUG: server who send response is: %s\n", tokens->server);
#endif
		}
		char* d_s = strstr(h_s,"Date: ");
		if(!d_s){
		  printf("No date header in the response\n");
		  strncpy(tokens->date,"Unknown",strlen("Unknown"));
		}else{
		  char* d_e = strstr(d_s,"\r\n");
		  //printf("DEBUG:s_e is:%s\n",s_e);
		  strncpy(tokens->date, d_s+6,(int)(d_e - d_s)-6);
#ifdef DEBUG
		  printf("DEBUG: date is: %s\n", tokens->date);
#endif
		}
		char* e_s = strstr(h_s, "Expires: ");
		//printf("DEBUG:e_s is:%s\n",e_s);
		if(!e_s){
		 printf("No expires header in the response\n");
		 strncpy(tokens->expire,"Unknown",strlen("Unknown"));
		}else{
		  char* e_e = strstr(e_s,"\r\n");
		  //printf("DEBUG:e_e is:%s\n",e_e);
		  strncpy(tokens->expire, e_s+9,(int)(e_e - e_s)-9);
#ifdef DEBUG
		  printf("DEBUG: expire date is: %s\n", tokens->expire);
#endif
		}
	      }else{
		printf("No header fields\n");
	      }
	    }else{
	      printf("status doesn't end with \\r\\n\n");
	      return NULL;
	    }
	  }else{
	    printf("Nothing after the status code\n");
	    return NULL;
	  }  
	}else{
	  printf("no space after version\n");
	  return NULL;
	}
      }else{
	printf("version not end with space\n");
	return NULL;
      }
    }else{
      printf("No protocol version in response\n");
      return NULL;
    }
    
    
  }
  return tokens;
}

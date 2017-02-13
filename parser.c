#include "parser.h"

#define DEBUG

char* buffer = "   CONNECT ftp://www.cmu.edu:8088/hub/index.html HTTP/1.1 lala \r\nthis is a bullshit header\r\nConnection:close\r\n\r\n";

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



/*extract information from the request_line buffer*/
/*req_sock = tokens->host, req_serv = tokens->prtc*/
char* parse_request(char* buffer){
#ifdef DEBUG
  printf("start parsing request:\n");
#endif
  req_info* tokens = (req_info*)malloc(sizeof(req_info));
  if(tokens == NULL){
    free(tokens);
    printf("Failed to malloc\n");
    return NULL;
  }
#ifdef DEBUG
  printf("malloced for tokens\n");
#endif
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
    printf("The input request line is:%s\n", m_s);
#endif
    char* m_e = strchr(m_s,' '); //first space after the start, should be the end of the method
    if(m_e){
      //printf("m_e point to %s\n", m_e);
      strncpy(tokens->method, m_s,(int)(m_e - m_s)); //copy into method[]
      i += (size_t)(m_e - m_s); 
      //  tokens->method[i+1] = '\0'; //end it by null terminator
#ifdef DEBUG
      printf("DEBUG: Method is: %s\n", tokens->method);
#endif
      //only accpet three method request
      if (strcasecmp(tokens->method, "GET") && strcasecmp(tokens->method, "POST")&& strcasecmp(tokens->method, "CONNECT")) {
	printf("This method cannot be implemented\n");
	return NULL;
      }
    }else{
      printf("There should be space after the request method\n");
      return NULL;
    }
  }else{
    printf("Please enter the request line\n");
    return NULL;
  }
  /*get the complete url*/
  char* c_s = strchr(m_s+strlen(tokens->method)-1,' '); 
  char* c_e = strchr(c_s+1, ' ');
  //printf("End of the url is:%s\n",c_e);
  // tokens->c_url=(char*)malloc(strlen(tokens->c_url) +1);
  strncpy(tokens->c_url, c_s+1, (int)(c_e - c_s));
#ifdef DEBUG
  printf("DEBUG: Complete url is:%s\n", tokens->c_url);
#endif
  //need to get the host and the protocol type
  char* p_s = c_s + 1; //start of the protocol ex.http, ftp, ...
  if(p_s){
    char* p_e = strstr(p_s + 1, "://"); //the colon before "//" ex.http://
    if(p_e){
      strncpy(tokens->prtc, p_s, (int)(p_e - p_s));
#ifdef DEBUG
      printf("DEBUG: Prtc is:%s\n", tokens->prtc);//copy into prtc[]
#endif
      //socket->serv = tokens->prtc;
    }
  }
  char* p_e = strchr(p_s + 1, ':'); 
  char* h_s = p_e + 3;//3 is the length of "://"
  while(IS_SPACE(*h_s)){
    h_s++;
  }
  if(h_s){
    char* po_s = strchr(h_s, ':');//after which is the port number if offered
    if(po_s){
#ifdef DEBUG
      printf("There is request port number in the url\n");
#endif
      char* po_e = strchr(po_s, '/'); //end of the host token, after the port number
      if(po_e){
	/*get the partial url from here*/
	char* pu_s = po_e;
	char* pu_e = strchr(pu_s, ' ');
	strncpy(tokens->p_url, pu_s, (int)(pu_e - pu_s)+1);
#ifdef DEBUG
	printf("DEBUG: The partial url is:%s\n", tokens->p_url);
#endif
	/*get the port*/
	char s_port[10];
	bzero(s_port,10); //fill with 0
	strncpy(tokens->host, h_s, (int)(po_s - h_s));
#ifdef DEBUG
	printf("DEBUG: Host is:%s\n", tokens->host);
#endif
	strncpy(s_port, po_s + 1, (int)(po_e - po_s)-1);
	//printf("DEBUG: Port is:%s\n", s_port);
	tokens->port = atoi(s_port);
#ifdef DEBUG
	printf("DEBUG: Port int is:%d\n",tokens->port);
#endif
      }else{
	printf("DEBUG:The url path is incomplete\n");
	return NULL;
      }
    }else{
      char* po_e = strchr(h_s, '/');
#ifdef DEBUG
      printf("DEBUG: There is no port field in request line, use default\n");
#endif
      strncpy(tokens->host, h_s, (int)(po_e - h_s) );
#ifdef DEBUG
      printf("DEBUG:Host is: %s\n",tokens->host);
#endif
      tokens->port = 80;
#ifdef DEBUG
      printf("DEBUG: port int is: %d\n", tokens->port);
#endif
    }
  }else{
    printf("DEBUG:Cannot find the start of the host in the request line\n");
    return NULL;
  }
#ifdef DEBUG
      printf("DEBUG:Request before rewrite:%s\n", buffer);
#endif
    /*Need to allocate a new string buffer for new request line with partial url and refilled headers*/
    char* http = " HTTP/1.1";
    char* request = (char*)malloc(sizeof(tokens->method) + sizeof(tokens->p_url) + sizeof(http) + sizeof(buffer));
    if(request == NULL){
      free(request);
      printf("Failed to malloc for request\n");
      return NULL;
    }
    //char* ptr = request;
    /*copy each fragments in request line into new buffer*/
    strncpy(request, tokens->method,strlen(tokens->method));
#ifdef DEBUG
    printf("after strncpy method:%s\n",request);
#endif
    strncpy(request+strlen(tokens->method),tokens->p_url,strlen(tokens->p_url));
#ifdef DEBUG
    printf("after strncpy p_url:%s\n",request);
#endif
    strncpy(request+strlen(tokens->method)+strlen(tokens->p_url),http,strlen(http));
#ifdef DEBUG
    printf("after strncpy protocol version:%s\n",request);
#endif
    char* l_e = request+strlen(tokens->method)+strlen(tokens->p_url)+strlen(http);
    if(!l_e){
      printf("request buffer ran out after first line\n");
      return NULL;
    }else{
    //printf("DEBUG:l_e is:%s\n", l_e);
      char* LRCF = "\r\n";
      // printf("DEBUG: LRCF is:%s\n", LRCF);
      char* hd_s = strstr(buffer,LRCF);
      //printf("DEBUG:hd_s is:%s\n", hd_s);
      if(!hd_s){
	printf("Request line doesn't end with LRCF\n");
	return NULL;
      }else{
	char* ho_s = strstr(hd_s, "Host:");
	//	printf("DEBUG:ho_s is:%s\n",ho_s);
	if(!ho_s){
	  char* h_str = "\r\nHost: ";
	 
#ifdef DEBUG
	  printf("DEBUG: No Host header field, need to be added as following\n");
	  printf("Host: %s\n", tokens->host);
#endif
	  strncpy(l_e,h_str,8);
	  strncpy(l_e + 8, tokens->host,strlen(tokens->host));
#ifdef DEBUG
	  printf("after adding host:%s\n",request);
#endif
	}
	 char* l_e2 = request + strlen(request);
	  // printf("DEBUG:request+strlen(request) is:%s\n",l_e2);
	 if(!l_e2){
	   printf("request buffer ran out\n");
	   return NULL;
	 }else{
	   //char*h_str = "\r\nConnection:close";
	   char* cn_s = strstr(buffer,"Connection:");
	   if(!cn_s){
	     char*h_str = "\r\nConnection:close"; 
#ifdef DEBUG
	     printf("DEBUG: No Connection header field, need to be added as following\n");
	     printf("Connection: close\n");
#endif
	     strncpy(l_e2,h_str,strlen(h_str));
#ifdef DEBUG
	     printf("after adding c:c is:%s\n",request);
#endif
	   }
	   char* l_e3 = request + strlen(request);
	   if(!l_e3){
	     printf("request buffer ran out\n");
	     return NULL;
	   }else{
	     char* pc_s = strstr(buffer,"Proxy-Connection:");
	     // printf("DEBUG:pc_s is:%s\n",pc_s);
	     if(!pc_s){
	       char* h_str = "\r\nProxy-Connection: close"; 
	       strncpy(l_e3, h_str,strlen(h_str));
#ifdef DEBUG
	       printf("after adding p-c:c is:%s\n",request);
#endif
	     }
	     char* l_e4 = request + strlen(request);
	     if(!l_e4){
	       printf("request buffer ran out\n");
	       return NULL;
	     }else{
	       char* u_a = strstr(buffer, "User-Agent:");
	       //   printf("DEBUG: u_a is:%s\n",u_a);
	       if(!u_a){
		 char* h_str = "\r\nUser-Agetn:Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3)\n  Gecko/20120305 Firefox/10.0.3";  
#ifdef DEBUG
		 printf("DEBUG: No User-Agent header field, need to be added as following\n");
		 printf("User-Agent: xxx\n");
#endif
		 strncpy(l_e4,h_str,strlen(h_str));
#ifdef DEBUG
		 printf("after adding u-a is:%s\n",request);
#endif
	       }
	     }
	   }
	 }
	 /*copy the original headers into new buffer*/
	 //char* start = buffer+strlen(request);
	 // printf("DEBUG:start is:%s\n",start);
	 char* end = strstr(buffer,"\r\n\r\n")+4;
	 //printf("DEBUG:end is:%s\n", end);
	 if(!end){
	   printf("The request doesn't end with \\r\\n\n");
	   return NULL;
	 }else{
	   char* header = request + strlen(request);
	   //printf("DEBUG:header is:%s\n",header);
	   strncpy(header, hd_s, (size_t)(end-hd_s));
	 }
#ifdef DEBUG
	 printf("after adding rest headers is:%s\n",request);
#endif	 
      }
    }
#ifdef DEBUG
    printf("after rewrite, return as:\n%s\n", request);
#endif

  return request;
}


/* void forward_header(int dest_sock){ */
/*   rewrite_header(); */
/*   //#ifdef DEBUG */
/*     // LOG("================ The Forward HEAD ================="); */
/*   //LOG("%s\n",header_buffer); */
/*   //#endif */
  
/*   int len = strlen(buffer); */
/*   send(dest_sock,buffer,len,0) ; */
/* } */

/* void handle_client(int client_sock, struct sockaddr_in client_addr,req_info tokens){ */
/*   int is_http_tunnel = 0;  */
/*   /\* if(strlen(tokens->host) == 0) { *\/ */
/*   /\*    if(read_header(client_sock,header_buffer) < 0){ *\/ */
/*   /\*      LOG("Read Http header failed\n"); *\/ */
/*   /\*      return; *\/ */
/*   /\*    } else { *\/ */
/*   /\*      char * p = strstr(header_buffer,"CONNECT"); /\\*connect request?*\\/ *\/ */
/*   /\*      if(p){ *\/ */
/*   /\* 	 LOG("receive CONNECT request\n"); *\/ */
/*   /\* 	 is_http_tunnel = 1; *\/ */
/*   /\*      } *\/ */
       
/*   if ((remote_sock = create_connection()) < 0) { */
/*     LOG("Cannot connect to host [%s:%d]\n",remote_host,remote_port); */
/*     return; */
/*   } */
  
/*   if (fork() == 0) { // 创建子进程用于从客户端转发数据到远端socket接口 */
    
/*     if(strlen(header_buffer) > 0 && !is_http_tunnel) { */
/* 	forward_header(remote_sock); //普通的http请求先转发header */
/*     }  */
    
/*     forward_data(client_sock, remote_sock); */
/*     exit(0); */
/*   } */
/* } */
  

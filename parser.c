#include "parser.h"

#define DEBUG

//char* buffer = "   CONNECT ftp://www.cmu.edu:8088/hub/index.html HTTP/1.1 lala \r\nthis is a bullshit header\r\nConnection:close\r\n\r\n";

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
char* parse_request(char* buffer, req_info* tokens){
  char* m_s = buffer;
  char* m_e = strchr(m_s,' '); 

  /* method */
  for(int i = 0; i < 10; i++){
	tokens->method[i] = '\0';
  }
  strncpy(tokens->method, m_s, (int)(m_e - m_s)); //copy into method[]
  if(strcasecmp(tokens->method, "GET") && strcasecmp(tokens->method, "POST") && strcasecmp(tokens->method, "CONNECT")) {
	printf("This method cannot be implemented\n");
	return NULL;
  }

  /* completed URL */
  m_s = m_e + 1;
  m_e = strchr(m_s,' '); 
  tokens->c_url = (char*)malloc((int)(m_e - m_s + 1)); 
  strncpy(tokens->c_url, m_s, (int)(m_e - m_s));
  strcat(tokens->c_url, "");

  /* protocol */
  m_e = strstr(m_s, "://"); 
  for(int i = 0; i < 10; i++){
  	tokens->prtc[i] = '\0';
  }
  strncpy(tokens->prtc, m_s, (int)(m_e - m_s));
  
  /* host, port, partial URL */
  m_s = m_e + 3; //3 is the length of "://"
  if(strchr(m_s, ':') != NULL && strchr(m_s, ':') < strchr(m_s, ' ')){ // has port e.g. www.google.com:8080/maps HTTP/1.1\r\n
  	/* host */
    m_e = strchr(m_s, ':');
    tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
	strncpy(tokens->host, m_s, (int)(m_e - m_s));
	strcat(tokens->host, "");
	m_s = m_e + 1;
    if(strchr(m_s, '/') != NULL && strchr(m_s, '/') < strchr(m_s, ' ')){ // has partial URL e.g. www.google.com:8080/maps HTTP/1.1\r\n
      /* port */
      m_e = strchr(m_s, '/'); 
      char* temp = malloc((int)(m_e - m_s + 1)); 
	  strncpy(temp, m_s, (int)(m_e - m_s));
	  strcat(temp, "");
      tokens->port = atoi(temp);
      free(temp);
      m_s = m_e;
	  
	  /* Partial URL */
	  m_e = strchr(m_s, ' ');
      tokens->p_url = (char*)malloc((int)(m_e - m_s + 1)); 
	  strncpy(tokens->p_url, m_s, (int)(m_e - m_s));
	  strcat(tokens->p_url, "");
    }
    else{ // no partial URL e.g. www.google.com:8080 HTTP/1.1\r\n
      /* port */
      m_e = strchr(m_s, ' '); 
      char* temp = malloc((int)(m_e - m_s + 1)); 
	  strncpy(temp, m_s, (int)(m_e - m_s));
	  strcat(temp, "");
      tokens->port = atoi(temp);
      free(temp);

      /* partial URL */
      tokens->p_url = (char*)malloc(2*sizeof(char));
      strcat(tokens->p_url, "/");
    }
  }
  else{ // no port
    if(strchr(m_s, '/') != NULL && strchr(m_s, '/') < strchr(m_s, ' ')){ // has partial URL e.g. www.google.com/maps HTTP/1.1
      /* Host */
      m_e = strchr(m_s, '/');
      tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
	  strncpy(tokens->host, m_s, (int)(m_e - m_s));
	  strcat(tokens->host, "");
	  m_s = m_e;
	  
	  /* Partial URL */
	  m_e = strchr(m_s, ' ');
      tokens->p_url = (char*)malloc((int)(m_e - m_s + 1)); 
	  strncpy(tokens->p_url, m_s, (int)(m_e - m_s));
	  strcat(tokens->p_url, "");
    }
    else{ //  no partial URL e.g. www.google.com HTTP/1.1
      /* Host */
      m_e = strchr(m_s, ' ');
      tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
	  strncpy(tokens->host, m_s, (int)(m_e - m_s));
	  strcat(tokens->host, "");

	  /* Port */
      tokens->port = 80;

      /* Partial URL */
      tokens->p_url = (char*)malloc(2*sizeof(char));
      strcat(tokens->p_url, "/");
    }
  }
  m_s = strstr(m_e, "\r\n");
  char* str_host = "\r\nHost: ";
  int need_host = 0;
  if(strstr(m_s, str_host) == NULL){
  	need_host = 1;
  }
  else if(strstr(m_s, str_host) > strstr(m_s, "\r\n\r\n")){
  	need_host = 1;
  }
  char* str_conn = "\r\nConnection: ";
  int need_conn = 0;
  if(strstr(m_s, str_conn) == NULL){
  	need_conn = 1;
  }
  else if(strstr(m_s, str_conn) > strstr(m_s, "\r\n\r\n")){
  	need_conn = 1;
  }
  char* http = "HTTP/1.1";
  char* clos = "Keep-Alive";
  size_t alloclen = strlen(tokens->method)+strlen(tokens->p_url)+strlen(http)+2 + need_host*(strlen(str_host)+strlen(tokens->host)) + need_conn*(strlen(str_conn)+strlen(clos)) + strlen(m_s)+1;
  char* ans = malloc(alloclen);
  if(ans == NULL){
  	printf("Malloc failed\n");
  	return NULL;
  }
  memset(ans, '\0', alloclen);
  strcpy(ans, tokens->method);
  strcat(ans, " ");
  strcat(ans, tokens->p_url);
  strcat(ans, " ");
  strcat(ans, http);
  if(need_host == 1){
  	strcat(ans, str_host);
  	strcat(ans, tokens->host);
  }
  if(need_conn == 1){
  	strcat(ans, str_conn);
  	strcat(ans, clos);
  }
  strcat(ans, m_s);
  return ans;
}


rsp_info* response_parser(char* buffer){
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

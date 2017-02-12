#include "http_parer.h"

/* get_from_connect connect; */
/* connect->remost_host = NULL; */
/* connect->remote_port = 0; */
/* connect->local_port = 0; */


char* global_buffer;

/*read from a socket to a buffer*/
ssize_t read_line_to_buf(int socket, void* buffer, size_t n){
  ssize_t numRead;
  size_t totRead;
  char *buf;
  char ch;
  
  if (n <= 0 || buffer == NULL) {
    errno = EINVAL; //invalid argument
    return -1;
  }
  
  buf = buffer;
  totRead = 0;
  while(1) {
    numRead = recv(socket, buffer, n, 0);
    if (numRead == -1) {
      if (errno == EINTR){  /* interuppted function call */
	continue;
      } else{
	return -1;
      }              /* unknown err */
    } else if (numRead == 0) {      /* EOF */
      if (totRead == 0){           /* No bytes read; return 0 */
	return 0;
      } else{                        /* Some bytes read; add '\0' */
	break;
      }
    } else {
      if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
	totRead++;
	*buf++ = ch;
      }
      if (ch == '\n'){
	break;
      }
    }
  }
  *buf = '\0';
  return totRead;
}


/*read the request line(ended with first \r\n) from global buffer(which read from client socket, contains all the message), then save to the line_buffer*/
int read_req_line(int socket, void* global_buffer){
  memset(header_buffer,0,MAX_HEADER_SIZE);//copy the first n byte with 0
  char line_buffer[4096]; //may need to realloc
  char * base_ptr = header_buffer;
  while(1){
    memset(line_buffer,0,4096); //fill the memory with 0
    ssize_t total_read =  read_line_to_buf(client_sock,line_buffer,4096);
    if(total_read <= 0){
      return CLIENT_SOCKET_ERROR;
    }
    //In case the header buffer overflow
    if(base_ptr + total_read - header_buffer <= MAX_HEADER_SIZE){
      strncpy(base_ptr,line_buffer,total_read);
      base_ptr += total_read;
    } else {
      return HEADER_BUFFER_FULL;
    }
    //read CRLF, header ends
    if(strcmp(line_buffer,"\r\n") == 0 || strcmp(line_buffer,"\n") == 0){
      break;
    }
  }
  return 0;
}

/*extract the host name and protocol type from the request_line buffer*/
req_info parse_request(char* buffer, req_sock socket){
  
  req_info tokens;
  tokens->method = NULL;
  tokens->prtc = NULL;
  tokens->host = NULL;
  tokens->c_url = NULL;
  tokens->p_url = NULL;
  tokens.port = 0;

  size_t i = 0;
  //skip space
  while(IS_SPACE(buffer[i])){
    i++;
  }
  if(i >= BUFFER_SIZE){
    printf("Too many spaces, out of buffer bound\n");
    return;
  }
  char* m_s = buffer[i]; //first non-space char, should be the start of the method
  if(m_s){
    char* m_e = strchr(m_s,' '); //first space after the start, should be the end of the method
    if(m_e){
      strncpy(tokens->method, m_s,(size_t)(m_e - m_s)); //copy into method[]
      i += (size_t)(m_e - m_s); 
      tokens->method[i+1] = '\0'; //end it by null terminator
      printf("DEBUG: Method is: %s\n", method);
      //only accpet three method request
      if (strcasecmp(tokens->method, "GET") && strcasecmp(tokens->method, "POST")&& strcasecmp(tokens->method, "CONNECT")) {
	printf("This method cannot be implemented\n");
	exit(EXIT_SUCCESS);
      }
    }
  }
  /*get the complete url*/
  char* c_e = strchr(m_e, ' ');
  strncpy(tokens->url, m_s, size_t(m_e - m_s));
  printf("DEBUG: Complete url is: %s\n", tokens->url);
  
  //need to get the host and the protocol type
    char* p_s = m_s + 1; //start of the protocol ex.http, ftp, ...
    if(p_s){
      char* p_e = strchr(p_s + 1, ':'); //the colon before "//" ex.http://
      if(p_e){
	strncpy(tokens->prtc, p_s, (size_t)(p_e - p_s));
	printf("DEBUG:Prtc is: %s\n", tokens->prtc);//copy into prtc[]
	socket->serv = tokens->prtc;
      }
    }
    char* h_s = p_e + 3;//3 is the length of "://"
    while(IS_SPACE(*h_s)){
      h_s++;
    }
    if(h_s){
      char* po_s = strchr(h_s, ':');//after which is the port number if offered
      if(po_s){
	char* po_e = strchr(po_s, '/'); //end of the host token, after the port number
	if(po_e){
	  /*get the partial url from here*/
	  char* pu_s = po_e;
	  char* pu_e = strchr(pu_s, ' ');
	  strncpy(tokens->p_url, pu_s, size_t(pu_e - pu_s)+1);
	  printf("DEBUG: The partial url is %s\n", tokens->p_url);
	  /*get the port*/
	  char s_port[10];
	  bzero(s_port,10); //fill with 0
	  strncpy(tokens->host, h_s, (size_t)(po_s - h_s) - 1);
	  printf("DEBUG: Host is: %s\n", tokens->host);
	  strncpy(s_port, po_s + 1, (size_t)(po_e - po_s) - 1);
	  printf("DEBUG: Port is: %s\n", s_port);
	  tokens.port = atoi(s_port);
	  printf("DEBUG: Port int is: %s\n",tokens.port);
	}else{
	  printf("DEBUG:The url path is incomplete\n");
	}
      }else{
	printf("DEBUG: There is no port field in request line, use default\n");
	strncpy(tokens->host, h_s, (size_t)(po_e - h_s) - 1);
	tokens->port = 80;
      }
    }else{
      printf("DEBUG:Cannot find the start of the host in the request line\n");
      // exit(EXIT_FAILURE);
    }
    /*No host in request line, go find the header*/
    char* h_h = strstr(header_buffer, "Host:");
    if(!h_h){
      printf("DEBUG: No Host header field, need to add\n");
      printf("\nHost: %s\n", tokens->host);
    }
    char* h_n = strchr(h_h, '\n');
    if(!h_n){
      printf("DEBUG:No newline in headers,bad protocol\n");
      exit(EXIT_SUCCESS);
    }
    /* char* h = strchr(h_h + 5, ':'); //length of host: */
    /* if(h && h < h_n){ */
    /*   size_t h_len = (size_t)(h_n - h - 1); */
    /*   char ss_port[p_len]; */
    /*   strncpy(ss_port, h+1,p_len); */
    /*   ss_port[p_len] = '\0'; */
    /*   tokens->port = atoi(ss_port); */
    /*   size_t h_len = (size_t)(h - h_h - 5 -1); */
    /*   strncpy(tokens->host, h_h + 5 +1, h_len); */
    /*   tokens->host[h_len] = '\0'; */
    /* }else{ */
    /*   size_t h_len = (size_t)(h_n - h_h - 5 - 1 - 1); */
    /*   strncpy(tokens->host, h_h + 5 + 1, h_len); */
    /*   tokens->host[h_len] = '\0'; */
    /*   tokens->port = 80; */
    /* } */
    socket->host = tokens->host;
    return tokens;
}

/*rewrite the complete url to the partial url*/
void rewrite_header(){
  char * p = strstr(header_buffer,"://");
  char * p0 = strchr(p,'\0');
  char * p5 = strstr(header_buffer,"HTTP/"); 
  int len = strlen(header_buffer);
  if(p){
    char * p1 = strchr(p + 7,'/');
    if(p1 && (p5 > p1)){
      //convert url to path
      memcpy(p,p1,(int)(p0 -p1));
      int l = len - (p1 - p) ;
      header_buffer[l] = '\0';
    }else{
      char * p2 = strchr(p,' ');  //GET http://3g.sina.com.cn HTTP/1.1
      // printf("%s\n",p2);
      memcpy(p + 1,p2,(int)(p0-p2));
      *p = '/';  //url has no path, use root dir
      int l  = len - (p2  - p ) + 1;
      header_buffer[l] = '\0';
     }
  }
}

void forward_header(int dest_sock){
  rewrite_header();
  //#ifdef DEBUG
    // LOG("================ The Forward HEAD =================");
  //LOG("%s\n",header_buffer);
  //#endif
  
  int len = strlen(header_buffer);
  send(dest_sock,header_buffer,len,0) ;
}

void handle_client(int client_sock, struct sockaddr_in client_addr,req_info tokens){
  int is_http_tunnel = 0; 
  /* if(strlen(tokens->host) == 0) { */
  /*    if(read_header(client_sock,header_buffer) < 0){ */
  /*      LOG("Read Http header failed\n"); */
  /*      return; */
  /*    } else { */
  /*      char * p = strstr(header_buffer,"CONNECT"); /\*connect request?*\/ */
  /*      if(p){ */
  /* 	 LOG("receive CONNECT request\n"); */
  /* 	 is_http_tunnel = 1; */
  /*      } */
       
  if ((remote_sock = create_connection()) < 0) {
    LOG("Cannot connect to host [%s:%d]\n",remote_host,remote_port);
    return;
  }
  
  if (fork() == 0) { // 创建子进程用于从客户端转发数据到远端socket接口
    
    if(strlen(header_buffer) > 0 && !is_http_tunnel) {
	forward_header(remote_sock); //普通的http请求先转发header
    } 
    
    forward_data(client_sock, remote_sock);
    exit(0);
  }
}
  

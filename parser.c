#include "parser.h"

#define DEBUG

//char* buffer = "   CONNECT ftp://www.cmu.edu:8088/hub/index.html HTTP/1.1 lala \r\nthis is a bullshit header\r\nConnection:close\r\n\r\n";



/*extract information from the request_line buffer*/
/*req_sock = tokens->host, req_serv = tokens->prtc*/
char* parse_request(char* buffer, req_info* tokens){
#ifdef DEBUG
  printf("start parsing request:\n");
#endif
  //  req_info* tokens = (req_info*)malloc(sizeof(req_info));
  //if(tokens == NULL){
  //free(tokens);
  // printf("Failed to malloc\n");
  //return NULL;
  // }
#ifdef DEBUG
  printf("get the malloced tokens as parameter\n");
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
      strncpy(tokens->method, m_s,(int)(m_e + 1 - m_s)); //copy into method[]
      // i += (size_t)(m_e - m_s); 
      //  tokens->method[i+1] = '\0'; //end it by null terminator
#ifdef DEBUG
      printf("DEBUG: Method is: %s\n", tokens->method);
#endif
      //only accpet three method request
      if (strcasecmp(tokens->method, "GET ") && strcasecmp(tokens->method, "POST ")&& strcasecmp(tokens->method, "CONNECT ")) {
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
  tokens->c_url = (char*)malloc((int)(c_e - c_s)); 
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
	tokens->p_url = (char*)malloc((int)(pu_e - pu_s) + 1);
	strncpy(tokens->p_url, pu_s, (int)(pu_e - pu_s)+1);
#ifdef DEBUG
	printf("DEBUG: The partial url is:%s\n", tokens->p_url);
#endif
	/*get the port*/
	char s_port[10];
	bzero(s_port,10); //fill with 0
	tokens->host = (char*)malloc((int)(po_s - h_s)); 
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
    }else{/*no explicit port number*/
      char* po_e = strchr(h_s, '/');
#ifdef DEBUG
      printf("DEBUG: There is no port field in request line, use default\n");
#endif
      /*get the partial url*/
      char* po_e = strchr(h_s,'/');
      if(po_e){
	/*get the partial url from here*/
	char* pu_s = po_e;
	char* pu_e = strchr(pu_s, ' ');
	tokens->p_url = (char*)malloc((int)(pu_e - pu_s) + 1);
	strncpy(tokens->p_url, pu_s, (int)(pu_e - pu_s)+1);
#ifdef DEBUG
	printf("DEBUG: The partial url is:%s\n", tokens->p_url);
#endif
	strncpy(tokens->host, h_s, (int)(po_e - h_s) );
#ifdef DEBUG
	printf("DEBUG:Host is: %s\n",tokens->host);
#endif
	tokens->port = 80;
#ifdef DEBUG
	printf("DEBUG: port int is: %d\n", tokens->port);
#endif
      }else{
	printf("DEBUG:The url path is incomplete\n");
	return NULL;
      }
    }
  }else{
    printf("DEBUG:Cannot find the start of the host in the request line\n");
    return NULL;
  }
#ifdef DEBUG
  printf("DEBUG:Request before rewrite:%s\n", buffer);
#endif
  /*Need to allocate a new string buffer for new request line with partial url and refilled headers*/
  char* http = "HTTP/1.1";
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
      strncpy(l_e, LRCF, 2);//add the first \r\n into new buffer
      char* end = strstr(buffer,"\r\n\r\n");
      //printf("DEBUG:end is:%s\n", end);
      if(!end){
	printf("The request doesn't end with \\r\\n\n");//
	return NULL;
      }else{
#ifdef DEBUG
	printf("DEBUG:The headers end with \\r\\n\\r\\n\n");
#endif
	/*copy the original headers into new buffer*/
	char* header = l_e+2;
	//printf("DEBUG:header-3 is:%s\n",header-3);
	  //printf("DEBUG:l_e+2-3 is:%s\n",l_e+2-3);
	char* buf_header = strstr(buffer,"\r\n");
	//printf("DEBUG:buf_header+2 is:'%s'\n",buf_header);
	strncpy(header, buf_header+2, (size_t)(end-buf_header-2));
	//printf("DEBUG:the copy end-4 is:'%s'\n", end-4);
	//printf("DEBUG:after copying whole headers the header is:%s\n", header);
      }
#ifdef DEBUG
      printf("after adding rest headers is:%s\n",request);
#endif
      char* hs_in_req = strstr(request,"\r\n");
      char* l_e1 = request + strlen(request);
      if(!l_e1){
      }else{
	char* ho_s = strstr(hs_in_req, "Host:");
	//	printf("DEBUG:ho_s is:%s\n",ho_s);
	if(!ho_s){
	  char* h_str = "\r\nHost: ";
#ifdef DEBUG
	  printf("DEBUG: No Host header field, need to be added as following\n");
	  printf("Host: %s\n", tokens->host);
#endif
	  strncpy(l_e1, h_str,8);
	  strncpy(l_e1+8, tokens->host,strlen(tokens->host));
#ifdef DEBUG
	    printf("after adding host request is:%s\n",request);
#endif
	}else{ 
#ifdef DEBUG
	  printf("DEBUG: Has Host header field, no need to be added\n");
#endif
	  }
      }
      char* l_e2 = request + strlen(request);
      // printf("DEBUG:request+strlen(request) is:%s\n",l_e2);
      if(!l_e2){
	printf("request buffer ran out\n");
	return NULL;
      }else{
	//char*h_str = "\r\nConnection:close";
	char* hs_in_req = strstr(request,"\r\n");  
	char* cn_s = strstr(hs_in_req,"Connection:");
	if((!cn_s) || (cn_s && (*(cn_s - 1) == '-'))){
	  char*h_str = "\r\nConnection:close"; 
#ifdef DEBUG
	  printf("DEBUG: No Connection header field, need to be added as following\n");
	  printf("Connection: close\n");
#endif
	  strncpy(l_e2,h_str,strlen(h_str));
#ifdef DEBUG
	  printf("after adding c:c request is:%s\n",request);
#endif
	}else{
#ifdef DEBUG
	  printf("DEBUG: Has c-c header field, no need to be added\n");
#endif
	}
      }
      char* l_e3 = request + strlen(request);
      //printf("DEBUG:l_e3 is:%s\n",l_e3);
      if(!l_e3){
	printf("request buffer ran out\n");
	return NULL;
      }else{
	char* hs_in_req = strstr(request,"\r\n");
	char* pc_s = strstr(hs_in_req,"Proxy-Connection:");
	// printf("DEBUG:pc_s is:%s\n",pc_s);
	if(!pc_s){
	  char* h_str = "\r\nProxy-Connection: close"; 
	  strncpy(l_e3, h_str,strlen(h_str));
#ifdef DEBUG
	  printf("DEBUG:after adding p-c:c is:%s\n",request);
#endif
	}else{
#ifdef DEBUG
	  printf("DEBUG: Has p-c header field, no need to be added\n");
#endif
	}
      }
      char* l_e4 = request + strlen(request);
      if(!l_e4){
	printf("request buffer ran out\n");
	return NULL;
      }else{
	char* hs_in_req = strstr(request,"\r\n");
	char* ua_s = strstr(hs_in_req, "User-Agent:");
	//   printf("DEBUG: u_a is:%s\n",u_a);
	if(!ua_s){
	  char* h_str = "\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3)";//\n  Gecko/20120305 Firefox/10.0.3";  
#ifdef DEBUG
	  printf("DEBUG: No User-Agent header field, need to be added as following\n");
	  printf("User-Agent: xxx\n");
#endif
	  strncpy(l_e4,h_str,strlen(h_str));
#ifdef DEBUG
	  printf("after adding u-a is:%s\n",request);
#endif
	}else{
#ifdef DEBUG
	  printf("DEBUG: Has u-a header field, no need to be added\n");
#endif
	}
      }
      char* l_e5 = request+ strlen(request);
      char* body_s = strstr(buffer,"\r\n\r\n");
      char* body_e = buffer + strlen(buffer);
      printf("DEBUG:body_e is:%s\n",body_e);
      if(!body_e){
	printf("buffer end with null\n");
	return NULL;
      }else{
	strncpy(l_e5,body_s,(int)(body_e - body_s));
#ifdef DEBUG
	printf("After copy body, request is:%s\n",request);
#endif
      }
    }
  }  
#ifdef DEBUG
  printf("after rewrite, return as:\n%s\n", request);
#endif
  
  //  free(tokens->c_url);
  // free(tokens->p_url);
  // free(tokens->host);
  
  return request;
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
		  tokens->date = (char*)malloc(strlen("Unknown"));
		  strncpy(tokens->date,"Unknown",strlen("Unknown"));
		}else{
		  char* d_e = strstr(d_s,"\r\n");
		  //printf("DEBUG:s_e is:%s\n",s_e);
		  tokens->date = (char*)malloc((int)(d_e - d_s)-6);
		  strncpy(tokens->date, d_s+6,(int)(d_e - d_s)-6);
#ifdef DEBUG
		  printf("DEBUG: date is: %s\n", tokens->date);
#endif
		}
		char* e_s = strstr(h_s, "Expires: ");
		//printf("DEBUG:e_s is:%s\n",e_s);
		if(!e_s){
		  printf("No expires header in the response\n");
		  
		  tokens->expire = (char*)malloc(strlen("Unknown"));       
		  strncpy(tokens->expire,"Unknown",strlen("Unknown"));
		}else{
		  char* e_e = strstr(e_s,"\r\n");
		  //printf("DEBUG:e_e is:%s\n",e_e);
		  tokens->expire = (char*)malloc((int)(e_e - e_s)-9);  
		  strncpy(tokens->expire, e_s+9,(int)(e_e - e_s)-9);
#ifdef DEBUG
		  printf("DEBUG: expire date is: %s\n", tokens->expire);
#endif
		}
		char* ca_s = strstr(h_s, "Cache-Control: ");
		if(!ca_s){
		  printf("No cache-control header in response\n");
		  strncpy(tokens->cache,"Unknown",strlen("Unknown"));
		}else{
		  char* ca_e = strstr(ca_s, "\r\n");
		  strncpy(tokens->cache, ca_s + strlen( "Cache-Control: "), (int)(ca_e - ca_s));
#ifdef DEBUG
		   printf("DEBUG: cache-control is: %s\n", tokens->cache);
#endif 
		}
		char* E_s = strstr(h_s, "Etag: ");
		if(!E_s){
		  printf("No Etag header in response\n");
		  tokens->Etag = (char*)malloc(strlen("Unknown"));
		  strncpy(tokens->Etag,"Unknown",strlen("Unknown"));
		}else{
		  char* E_e = strstr(E_s, "\r\n");
		  tokens->Etag = (char*)malloc((int)(E_e - E_s)-6);
		  strncpy(tokens->Etag, E_s+6,(int)(E_e - E_s)-6);
#ifdef DEBUG
		  printf("DEBUG:Etag is:%s\n",tokens->Etag);
#endif
		}
		char* cn_s = strstr(h_s,"Connection: ");
		if(!cn_s){
		  printf("No connection header in response\n");
	
		  strncpy(tokens->connection,"Unknown",strlen("Unknown"));
		}else{
		  char* cn_e = strstr(cn_s,"\r\n");
	
		  strncpy(tokens->connection, cn_s+12,(int)(cn_e - cn_s)-12);
#ifdef DEBUG
		  printf("DEBUG:Connection is:%s\n",tokens->Connection);
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

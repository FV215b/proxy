#include "cache.h"
#define DEBUG

char* getLoctime(){
  time_t curtime;
  struct tm* loc_time;
  curtime = time(NULL);
  loc_time = localtime(&curtime);
  char* curr = asctime(loc_time);
#ifdef DEBUG
  printf("getLoctime is:%s\n", curr);
#endif
  char* time = malloc(strlen(curr)+1);
  memset(time, '\0', strlen(curr)+1);
  strcpy(time, curr);
  return time; 
}

double expMinusDate(char* date, char* expire){
  struct tm d;
  struct tm e;
  strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &d);
  strptime(expire, "%a, %d %b %Y %H:%M:%S GMT", &e);
  time_t date_t = mktime(&d);  
  time_t expire_t = mktime(&e);  
  double diff = difftime(expire_t, date_t);
#ifdef DEBUG
  printf("expire - date in seconds is: %f\n", diff);
#endif
  return diff;
}

double curMinusDate(char* date){
  char* curr = getLoctime();
  struct tm c;
  strptime(curr, "%a, %b %d %H:%M:%S %Y", &c);
  struct tm d;
  strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &d);
  time_t date_t = mktime(&d);
  time_t curr_t = mktime(&c);
  double diff = difftime(curr_t, date_t);
#ifdef DEBUG
  printf("curr - date in seconds is: %f\n", diff);
#endif
  return diff;  
}


cache* head = NULL;
cache* tail = NULL;
int obj_num = 0;

bool allocCache(char* buff, char* url, int extime){
	int buffsize = strlen(buff)+1;
	if(buffsize > MAX_OBJECT_SIZE){
		printf("Cache response failed. Response size = %d bytes is too big and won't be cached\n", buffsize);
		return false;
	}
	if(obj_num >= MAX_OBJECT_NUM){
		deleteCache(tail);
	}
	obj_num += 1;
	cache* node = (cache *)malloc(sizeof(cache));
	memset(node, '\0', sizeof(cache));
	node->url = (char *)malloc(strlen(url)+1);
	memset(node->url, '\0', strlen(url)+1);
	strcpy(node->url, url);
	node->res = (char *)malloc(buffsize);
	memset(node->res, '\0', buffsize);
	strcpy(node->res, buff);
	node->ext = extime;
	cache* temp = head;
	if(head != NULL){
		head->prev = node;
	}
	else{
		tail = node;
	}
	head = node;
	head->next = temp;
	head->prev = NULL;
	printf("Cache response successfully\n");
	return true;
}

/* 
	1. Read for buffer size, if larger than threshold, then won't exist in cache
	2. Scan for cache, if found and return cached response
	3. If not found, allocate a memory and store response
*/
char* readCache(char* url){
	if(!scanCache(url)){
		printf("Cached response not found\n");
		return NULL;
	}
	printf("Found cached response\n");
	printf("%s\n", head->res);
	return head->res;
}

/* Scan for: 
	1. check if any URL matches(main)
	2. check if any cache expires(peripheral) 
*/
bool scanCache(char* url){
	cache* curr = head;
	bool found = false;
	while(curr != NULL){
		curr->ext -= 1;
		if(curr->ext <= 0){
			cache* temp = curr;
			curr = curr->next;
			deleteCache(temp);
			continue;
		}
		if(strcmp(curr->url, url) == 0){
			cache* temp = curr;
			curr = curr->next;
			moveCache(temp);
			found = true;
			continue;
		}
		curr = curr->next;
	}
	return found;
}

/* Move hit cache to the head */
void moveCache(cache* curr){
	if(head == curr){
		return;
	}
	cache* pre = curr->prev;
	cache* suf = curr->next;
	pre->next = suf;
	if(suf != NULL){
		suf->prev = pre;
	}
	else{
		tail = pre;
	}
	cache* temp = head;
	head->prev = curr;
	head = curr;
	head->next = temp;
	head->prev = NULL;
}

void deleteCache(cache* curr){
	if(obj_num == 0){
		return;
	}
	cache* pre = curr->prev;
	cache* suf = curr->next;
	if(pre != NULL){
		pre->next = suf;
	}
	else{
		head = suf;
	}
	if(suf != NULL){
		suf->prev = pre;
	}
	else{
		tail = pre;
	}
	free(curr->url);
	free(curr->res);
	free(curr);
	obj_num -= 1;
}

void printCache(){
	cache* curr = head;
	printf("Cache number: %d\n", obj_num);
	while(curr != NULL){
		printf("%s: %s\nexp: %d\n", curr->url, curr->res, curr->ext);
		curr = curr->next;
	}
}

/*int main(int argc, char const *argv[]){
	allocCache("1st hello", "www.bing.com/musics", EXPIRE_TIME);
	printCache();
	readCache("www.bing.com/musics");
	printCache();
	readCache("www.bing.com/musics");
	allocCache("1st hello", "www.bing.com/musics", EXPIRE_TIME);
	allocCache("2nd hello", "www.google.com/maps", EXPIRE_TIME);
	allocCache("3rd hello", "www.baidu.com/news", EXPIRE_TIME);
	allocCache("4th hello", "www.yahoo.com/musics", EXPIRE_TIME);
	printCache();
	readCache("www.baidu.com/news");
	printCache();
	allocCache("5th hello", "www.yahoo.com/images", EXPIRE_TIME);
	printCache();
	allocCache("6th hello", "www.zhihu.com/questions", EXPIRE_TIME);
	printCache();
	readCache("www.zhihu.com/questions");
	printCache();
	allocCache("longer hello", "www.zhihu.com/questions", EXPIRE_TIME);
	printCache();
	readCache("www.zhihu.com/questions");
	printCache();
	return EXIT_SUCCESS;
}*/

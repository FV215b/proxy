CFLAGS = -Wall -Werror -pedantic -std=gnu99

serverSocket: serverSocket.o parser.o cache.o socket.o cache_policy.o log.o
	gcc -o $@ $(CFLAGS) serverSocket.o parser.o cache.o socket.o cache_policy.o log.o
serverSocket.o: serverSocket.c
	gcc $(CFLAGS) -c serverSocket.c
parser.o: parser.c parser.h
	gcc $(CFLAGS) -c parser.c
cache.o: cache.c cache.h
	gcc $(CFLAGS) -c cache.c
socket.o: socket.c socket.h
	gcc $(CFLAGS) -c socket.c
cache_policy.o: cache_policy.c cache_policy.h
	gcc $(CFLAGS) -c cache_policy.c
log.o: log.c log.h
	gcc $(CFLAGS) -c log.c
.PHONY: clean
clean:
	rm -f parser.o serverSocket.o cache.o socket.o cache_policy.o log.o serverSocket 

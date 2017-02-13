CFLAGS = -Wall -Werror -pedantic -std=gnu99

serverSocket: serverSocket.o parser.o
	gcc -o $@ $(CFLAGS) serverSocket.o parser.o
serverSocket.o: serverSocket.c
	gcc $(CFLAGS) -c serverSocket.c
parser.o: parser.c parser.h
	gcc $(CFLAGS) -c parser.c
.PHONY: clean
clean:
	rm -f parser.o serverSocket.o serverSocket
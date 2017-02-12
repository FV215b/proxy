CFLAGS = -Wall -Werror -pedantic -std=gnu99

cache: cache.o
	gcc -o $@ $(CFLAGS) $<
cache.o: cache.c proxy.h
	gcc $(CFLAGS) -c cache.c
.PHONY: clean
clean:
	rm -f cache cache.o
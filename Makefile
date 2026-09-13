CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

.PHONY: all debug release clean

all: release

release: astring

debug: astringd

astring: astring.c astring.h contracts.h
	$(CC) $(CFLAGS) -o $@ astring.c

astringd: astring.c astring.h astring_test.c contracts.h
	$(CC) $(CFLAGS) -g -o0 -DDEBUG -o $@ astring.c astring_test.c

clean:
	rm -f astring astringd
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

.PHONY: all debug release clean

all: release

release: astring

debug: astringd

astring: astring.c contracts.h
	$(CC) $(CFLAGS) -o $@ astring.c

astringd: astring.c contracts.h
	$(CC) $(CFLAGS) -DDEBUG -o $@ astring.c

clean:
	rm -f astring astringd
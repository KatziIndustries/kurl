CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lws2_32

build:

test:
	$(CC) lib/*.c src/test.c -o kurl.exe $(CFLAGS) $(LDFLAGS)

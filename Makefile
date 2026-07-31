CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

ifeq ($(OS),Windows_NT)
LDFLAGS = -lws2_32
endif

build:

test:
	$(CC) lib/*.c src/test.c -o kurl $(CFLAGS) $(LDFLAGS)

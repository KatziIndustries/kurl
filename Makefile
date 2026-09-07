CC = gcc
AR = ar

CFLAGS = -Wall -Wextra -Ilib
LDFLAGS =

ifeq ($(OS),Windows_NT)
	LDFLAGS = -lws2_32
endif

LIB_NAME = libkurl.a

LIB_SRC = $(wildcard lib/*.c)
LIB_OBJ = $(LIB_SRC:.c=.o)


all: $(LIB_NAME)


$(LIB_NAME): $(LIB_OBJ)
	$(AR) rcs $@ $^


lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@


test: $(LIB_NAME)
	$(CC) $(CFLAGS) src/test.c -L. -lkurl -lssl -lcrypto -o kurl $(LDFLAGS)


clean:
	rm -f lib/*.o $(LIB_NAME) kurl

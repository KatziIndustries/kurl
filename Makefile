CC = gcc

CFLAGS = -Wall -Wextra -fPIC -O3
LDFLAGS = -Ilib
LDLIBS = -lssl -lcrypto

ifeq ($(OS),Windows_NT)
	LDFLAGS = -lws2_32
endif

LIB_NAME = libkurl.so

LIB_SRC = $(wildcard lib/*.c)
LIB_OBJ = $(LIB_SRC:.c=.o)


all: $(LIB_NAME)


$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $@ $(LIB_OBJ) $(LDLIBS)

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@


test: $(LIB_NAME)
	$(CC) $(CFLAGS) src/test.c -L. -lkurl $(LDLIBS) $(LDFLAGS) -o kurl


clean:
	rm -f lib/*.o $(LIB_NAME) kurl

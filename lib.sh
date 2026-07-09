#!/bin/sh
set -e

mkdir -p build

# Compile object files
gcc -c lib/*.c -Ilib

# Create a static library
ar rcs build/libkurl.a *.o

# Optional: create a shared library
gcc -shared -o build/libkurl.so *.o -lws2_32

# Cleanup
rm -f *.o
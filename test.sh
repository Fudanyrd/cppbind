#!/usr/bin/sh

set -ex

gcc -x c++ - < include/cppbind.h -std=c++11 -I/usr/include/python3.10 -Iinclude -c -o tmp.o
clang -x c++ - < include/cppbind.h -std=c++11 -I/usr/include/python3.10 -Iinclude -c -o tmp.o

gcc -x c++ - < include/cppbind.h -std=c++17 -I/usr/include/python3.10 -Iinclude -c -o tmp.o
clang -x c++ - < include/cppbind.h -std=c++17 -I/usr/include/python3.10 -Iinclude -c -o tmp.o
rm -f tmp.o || true

exit 0


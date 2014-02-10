.PHONY: all

all: libttu.so

libttu.so: src/ttu.c
	$(CC) src/ttu.c -ldl -shared -fPIC -o libttu.so

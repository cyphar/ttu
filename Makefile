.PHONY: all

all: clean libttu.so

libttu.so: src/ttu.c src/ohmic.c
	$(CC) src/ttu.c src/ohmic.c -ldl -Iinclude/ -shared -fPIC -o libttu.so

clean:
	rm -rf libttu.so

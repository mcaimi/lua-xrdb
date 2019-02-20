CC=gcc
CFLAGS=-Wall -shared -fPIC -I . `pkg-config --cflags --libs x11`

all: build53

build51:
	$(CC) $(CFLAGS) -I/usr/include/lua5.1 -llua5.1 -o xresources51.so xresources_bridge.c

build51_debug:
	$(CC) $(CFLAGS) -D__DEBUG -I/usr/include/lua5.1 -llua5.1 -o xresources51.so xresources_bridge.c

build52:
	$(CC) $(CFLAGS) -I/usr/include/lua5.2 -llua5.2 -o xresources52.so xresources_bridge.c

build52_debug:
	$(CC) $(CFLAGS) -D__DEBUG -I/usr/include/lua5.2 -llua5.2 -o xresources52.so xresources_bridge.c

build53:
	$(CC) $(CFLAGS) -llua5.3 -o xresources53.so xresources_bridge.c

build53_debug:
	$(CC) -D__DEBUG $(CFLAGS) -llua5.3 -o xresources53.so xresources_bridge.c

CPPFLAGS = -g

.PHONY: all build run clean

all: build

build: server subscriber

server: serverPoll.c
	gcc $(CPPFLAGS) -o server serverPoll.c

subscriber: clientTcp.c
	gcc $(CPPFLAGS) -o subscriber clientTcp.c

run: server subscriber
	./server arg1 &
	./subscriber arg2 arg3 arg4

clean:
	rm -f server subscriber
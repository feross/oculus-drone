CC = g++
CFLAGS = -g -Wall
LDFLAGS =

all: example

example: server.o server-socket.o
	$(CC) server.o server-socket.o $(LDFLAGS) -o example

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f main.o example

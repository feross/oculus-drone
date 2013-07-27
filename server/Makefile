CC = g++
CFLAGS = -g -Wall -I../include
LDFLAGS =

all: example

example: time-server-sequential.o server-socket.o
	$(CC) time-server-sequential.o server-socket.o $(LDFLAGS) -o example

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f main.o example

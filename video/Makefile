CC = gcc
CFLAGS = -g -Wall -I/opt/local/include
LDFLAGS = -L/opt/local/lib -lavcodec -lavutil

all: example

example: video2.o
	$(CC) video2.o $(LDFLAGS) -o example

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f video.o example

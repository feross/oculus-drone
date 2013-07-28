CC = gcc
CFLAGS = -g -Wall -I/opt/local/include
LDFLAGS = -L../ffmpeg -lffmpeghack

all: example

example: video.o
	$(CC) video.o $(LDFLAGS) -o example

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f video.o example

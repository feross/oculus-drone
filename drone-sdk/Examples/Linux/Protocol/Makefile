include custom.makefile

CFLAGS = -g -Wall
LDFLAGS =-lpthread 

ifeq ($(USE_OPENGL),yes)
CFLAGS+=-DBUILD_OGLES
LDFLAGS+=-lEGL
endif

CC = gcc
INCLUDES =
TARGET=ardrone_demo

GENERIC_LIBRARY_SOURCE_FILES=   \
	app.c \
	at_cmds.c \
	navdata.c \
	stream.c \
	vlib.c \
   main.c

ifeq ($(USE_OPENGL),yes)
GENERIC_LIBRARY_SOURCE_FILES+=   \
   video.c \
   default.c
endif

OBJS=$(GENERIC_LIBRARY_SOURCE_FILES:.c=.o)

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ -c $<

clean:
	rm *.o ~*

# DO NOT DELETE


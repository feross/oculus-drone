include custom.makefile

ifeq ($(USE_MINGW32),yes)
GENERIC_COMMAND_PREFIX=i586-mingw32msvc-
ifeq ($(shell which $(GENERIC_COMMAND_PREFIX)gcc 2> /dev/null),)
$(error ERROR You need Minimalist GNU Win32 cross compiler. (under Debian: apt-get install mingw32))
endif
endif

CFLAGS=-g -Wall
ifeq ($(USE_MINGW32),yes)
CFLAGS+=-DUSE_MINGW32
endif

ifeq ($(USE_MINGW32),yes)
LDFLAGS =             \
-lws2_32
else
LDFLAGS =             \
-lpthread
endif

ifeq ($(USE_OPENGL),yes)
CFLAGS+=-DBUILD_OGLES
LDFLAGS+=-lEGL
endif

CC = $(GENERIC_COMMAND_PREFIX)gcc
TARGET=protocol_demo

ifeq ($(USE_MINGW32),yes)
   OS=win32
else
   OS=linux
endif
INCLUDES=-I$(ALL_SOURCES)
INCLUDES+=-I$(ALL_SOURCES)/VP_Os
INCLUDES+=-I$(ALL_SOURCES)/VP_Os/$(OS)

GENERIC_LIBRARY_SOURCE_FILES=   \
   VP_Os/vp_os_malloc.c        \
	app.c \
	at_cmds.c \
	default.c \
	navdata.c \
	stream.c \
	vlib.c \
   main.c

GENERIC_LIBRARY_SOURCE_FILES+=            \
   VP_Os/$(OS)/vp_os_signal.c        \
   VP_Os/$(OS)/vp_os_delay.c         \
   VP_Os/$(OS)/vp_os_thread.c

ifeq ($(USE_OPENGL),yes)
GENERIC_LIBRARY_SOURCE_FILES+=   \
   video.c
endif

OBJS=$(GENERIC_LIBRARY_SOURCE_FILES:.c=.o)
all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}


.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ -c $<

clean:
	rm *.o

# DO NOT DELETE


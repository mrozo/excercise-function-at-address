.PHONY: gdb clean

CC=gcc
GDB=gdb
CFLAGS=-Werror
DBG_CFLAGS=$(CFLAGS) -ggdb -DDEBUG

SOURCES=addr2func.c
OBJECTS=$(patsubst %.c,build/%.o,$(SOURCES))
DEBUG_OBJECTS=$(patsubst build/%,debug/%,$(OBJECTS))
DUMMY_VAR:=$(shell mkdir -p ./build/ ./debug/)

build/%.o:$(SOURCES)
	$(CC) -o $@ $< $(CFLAGS)

all:$(OBJECTS)

debug/%.o:$(SOURCES)
	$(CC) -o $@ $< $(DBG_CFLAGS)

debug:$(DEBUG_OBJECTS)

gdb:debug
	chmod u+x ./debug/addr2func.o
	$(GDB) ./debug/addr2func.o

clean:
	rm -rf ./build/ ./debug/


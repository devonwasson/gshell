CC = gcc
CFLAGS = -std=gnu99 -Wall -g

EXECS = gshell dnode dlist

all: $(EXECS)

dnode: dnode.h dnode.c
	$(CC) $(CFLAGS) -c dnode.c -o dnode.o

dlist: dlist.h dlist.c
	$(CC) $(CFLAGS) -c dlist.c -o dlist.o

gshell: gshell.c dlist.o dnode.o
	$(CC) $(CFLAGS) dnode.o dlist.o gshell.c -o gshell

.PHONY: clean
clean:
	/bin/rm -rf *~ gshell dnode.o dlist.o

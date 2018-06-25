CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
fileindex: fileindex.c linebuffer.c
	$(CC) $(CFLAGS) -o fileindex fileindex.c linebuffer.c 

run: fileindex
	./fileindex joendhard.mbox


clean:
	rm fileindex

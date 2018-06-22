CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
fileindex: fileindex.c linebuffer.c
	$(CC) $(CFLAGS) -o fileindex fileindex.c linebuffer.c 

run: fileindex
	./fileindex joendhard_small.mbox

debug:
	nemiver ./fileindex joendhard_small.mbox
	

clean:
	rm fileindex

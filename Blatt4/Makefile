CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
fileindex: fileindex.c linebuffer.c
	$(CC) $(CFLAGS) -o fileindex fileindex.c linebuffer.c 

run: fileindex
	./fileindex testdatei_MIT_RN.txt


clean:
	rm fileindex

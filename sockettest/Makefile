CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client
	
run: server
	./server

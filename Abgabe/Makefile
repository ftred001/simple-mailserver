CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
database: database.c
	$(CC) $(CFLAGS) database.c -o database
	
dbm: dbm.c database.c
	$(CC) $(CFLAGS) -o dbm dbm.c database.c 

run: dbm
	./dbm


clean:
	rm dbm

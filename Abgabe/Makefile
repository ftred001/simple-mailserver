CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
database: database.c
	$(CC) $(CFLAGS) database.c -o database
	
dbm: dbm.c database.c dialog.c
	$(CC) $(CFLAGS) -o dbm dbm.c database.c dialog.c
	
pop: pop3.c dbm.c
	$(CC) $(CFLAGS) -o pop3 pop3.c dbm.c

run: dbm
	./dbm


clean:
	rm dbm
	rm pop3

CC = gcc
CFLAGS = -pthread -g -ansi -pedantic -Wall
		
dbm: dbm.c database.c dialog.c
	$(CC) $(CFLAGS) -o dbm dbm.c database.c dialog.c
	
mailserver: mailserver.c pop3.c database.c linebuffer.c fileindex.c dialog.c smtp.c
	$(CC) $(CFLAGS) -o mailserver mailserver.c pop3.c database.c linebuffer.c fileindex.c dialog.c smtp.c


run: mailserver
	./mailserver


clean:
	rm mailserver
	rm dbm
	rm mailserver.db


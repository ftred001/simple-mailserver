CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
database: database.c
	$(CC) $(CFLAGS) database.c -o database
	
dbm: dbm.c database.c dialog.c
	$(CC) $(CFLAGS) -o dbm dbm.c database.c dialog.c
	
pop3: pop3.c dialog.c linebuffer.c database.c fileindex.c
	$(CC) $(CFLAGS) -o pop3 pop3.c dialog.c linebuffer.c database.c fileindex.c
	
testlb: testlb.c linebuffer.c
	$(CC) $(CFLAGS) -o testlb testlb.c linebuffer.c
	
mailserver: mailserver.c pop3.c database.c linebuffer.c fileindex.c dialog.c
	$(CC) $(CFLAGS) -o mailserver mailserver.c pop3.c database.c linebuffer.c fileindex.c dialog.c

testclient: testclient.c
	$(CC) $(CFLAGS) testclient.c -o testclient

run: mailserver
	./mailserver


clean:
	rm pop3
	rm dbm
	rm mailserver
	rm database
	rm dbm
	rm testlb

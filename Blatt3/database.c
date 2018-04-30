#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> /* Folie 73 */
#include <unistd.h> /* Folie 77 */
#include <sys/types.h> /* Folie 80 */
#include "database.h"


int db_list(const char *path, int outfd, 
	int (*filter)(DBRecord *rec, const void *data),
	const void *data) {
	 return 0;
}

int speichern(int fd, DBRecord *dbr) {
	if (write(fd, dbr, sizeof(DBRecord)) < 0 ) {
		perror("speichern (write)\n");
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int lese_fd, schreib_fd;
	unsigned anzahl;
	char *daten;
	DBRecord test = {"key1", "cat1", "value toll"};
	DBRecord test2 = {"key2 ist länger", "Kategoriename", "Values gehen mir auf den sack!"};
	
	if (argc!=3) { printf("Falscher Aufruf\n"); exit(1); }
	
	
	schreib_fd = open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0644);
	if (schreib_fd < 0) {
		perror("Bei Oeffnen der Ausgabedatei");
		exit(3);
	}
	speichern(schreib_fd, &test);
	speichern(schreib_fd, &test2);
	close(schreib_fd);
	
	lese_fd = open(argv[1], O_RDONLY);
	if (lese_fd < 0) {
		perror("Bei Oeffnen der Eingabedatei");
		exit(2);
	}
	
	read(lese_fd, daten, anzahl);
	printf("Eingelesener String %s\n Anzahl Bytes: %d\n",daten, anzahl);
	
	close(lese_fd);
	
	return 0;
}

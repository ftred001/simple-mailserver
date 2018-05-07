#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> /* Folie 73 */
#include <unistd.h> /* Folie 77 */
#include <sys/types.h> /* Folie 80 */
#include <sys/mman.h> /*Folie 86 */
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

int db_search(const char *filepath, int start, DBRecord *record) {
	/* returns index if successful, returns -1 if not found, -42 if not found */
	int map_fd, laenge, i;
	DBRecord *maprecord;
	

	map_fd = open(filepath, O_RDWR, 0644);
	laenge = lseek(map_fd, 0, SEEK_END);
	
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, map_fd, 0);
	
	for (i=0; i<5; i++) {
		if (!strcmp(record->key, maprecord[i].key)) {
			printf("\n KEY Match: %s %s\n", record->key, maprecord[i].key);
			return i;
		}
		if (!strcmp(record->cat, maprecord[i].cat)) {
			printf("\n CAT Match: %s %s\n", record->cat, maprecord[i].cat);
			return i;
		}
	}
	munmap(maprecord, laenge);
	return -1;
}

int db_get(const char *filepath, int index, DBRecord *result) {
	/* returns 0 if successful. else -1 */
	int fd, laenge;
	DBRecord *maprecord;
	
	fd = open(filepath, O_RDWR, 0644);
	laenge = lseek(fd, 0, SEEK_END);
	
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	printf("GET INDEX %d : %s %s %s\n", index, maprecord[index].key, maprecord[index].cat, maprecord[index].value);

	strcpy(result->key, maprecord[index].key);
	strcpy(result->cat, maprecord[index].cat);
	strcpy(result->value, maprecord[index].value);
	
	printf("GET Result: %s %s %s\n", result->key, result->cat, result->value);

	return 0;
}

int main(int argc, char *argv[]) {
	int schreib_fd;
	int map_fd, laenge, i;
	DBRecord test = {"key1", "cat1", "value toll"};
	DBRecord test2 = {"key2 ist länger", "Kategoriename", "Values sind wirklich super!"};
	DBRecord *maprecord;
	DBRecord matchrecord = {"key1", "Kategoriename", "Toller Value"};
	DBRecord *match = &matchrecord;
	
	DBRecord index_result;
	
	
	if (argc!=3) { printf("Falscher Aufruf\n"); exit(1); }
	
	/* Schreibe Testdaten in Datei */
	schreib_fd = open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0644);
	if (schreib_fd < 0) {
		perror("Bei Oeffnen der Ausgabedatei");
		exit(3);
	}
	speichern(schreib_fd, &test);
	speichern(schreib_fd, &test2);
	close(schreib_fd);
	/* ENDE Schreibe Testdaten in Datei ENDE*/
	
	/* Einlesen per mmap(). */
	map_fd = open("hier", O_RDWR, 0644);
	laenge = lseek(map_fd, 0, SEEK_END);
	
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, map_fd, 0);
	
	for (i=0; i<5; i++) {
		printf("Key: %s, Cat: %s, Value: %s\n", maprecord[i].key, maprecord[i].cat, maprecord[i].value);
	}
	munmap(maprecord, laenge);
	/* ENDE Einlesen per mmap() ENDE) */
	
	/* In Datenbank nach Eintrag suchen */
	db_search("hier", 0, match);

	/* DB-Eintrag zu Index ausgeben */
	db_get("hier", 0, &index_result);
		
	
	return 0;
}

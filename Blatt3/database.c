#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> /* Folie 73 */
#include <unistd.h> /* Folie 77 */
#include <sys/types.h> /* Folie 80 */
#include <sys/mman.h> /*Folie 86 */
#include "database.h"


int speichern(int fd, DBRecord *dbr) {
	if (write(fd, dbr, sizeof(DBRecord)) < 0 ) {
		perror("speichern (write)\n");
		return -1;
	}
	return 0;
}

int match_filter(DBRecord *rec, const void *data) {
	if (!strcmp(rec->key, data)) {
		return 1;
	}
	if (!strcmp(rec->cat, data)) {
		return 1;
	}
	return 0;
}


int db_search(const char *filepath, int start, DBRecord *record) {
	/* returns index if successful, returns -1 if not found, -42 if not found */
	int map_fd, laenge, i;
	unsigned long data_count;
	DBRecord *maprecord;
	
	map_fd = open(filepath, O_RDWR, 0644);
	laenge = lseek(map_fd, 0, SEEK_END);
	
	data_count = laenge / sizeof(DBRecord);
	
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, map_fd, 0);
	
	for (i=0; i<data_count; i++) {
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
	
	printf("GET Result: %s %s %s\n\n", result->key, result->cat, result->value);

	return 0;
}

int db_list(const char *path, int outfd, 
  int (*filter)(DBRecord *rec, const void *data), const void *data) {
	/* Liest Datenbank ein und schreibt Inhalt als menschenlesbare Tabelle in outfd. */
	int in_fd, laenge,i,s;
	int key_rest, cat_rest, value_rest;
	unsigned long data_count;
	DBRecord *maprecord;
	char *zeile = malloc(sizeof(DBRecord)+ 8);
	char *separator = " | ";
	
	in_fd = open(path, O_RDONLY);
	laenge = lseek(in_fd,0, SEEK_END);
	
	data_count = laenge / sizeof(DBRecord);
	
	printf("Anzahl Datensätze: %ld\n\n", data_count);
	
	maprecord = mmap(0, laenge, PROT_READ, MAP_SHARED, in_fd, 0);
	
	
	/* Format List */
	for (i=0; i<data_count; i++) {
		/* Wenn Filter-Regeln stimmen, dann mache folgendes: */
		if (filter(&maprecord[i],data)) {
			zeile = strcat(zeile, "| ");
			/* Key */
			zeile = strcat(zeile, maprecord[i].key);
			key_rest = DB_KEYLEN - strlen(maprecord[i].key);
			for (s=0; s<key_rest; s++) {
				zeile = strcat(zeile, " ");
			}
			zeile = strcat(zeile, separator);
			
			/* Cat */
			zeile = strcat(zeile, maprecord[i].cat);
			cat_rest = DB_CATLEN - strlen(maprecord[i].cat);
			for (s=0; s<cat_rest; s++) {
				zeile = strcat(zeile, " ");
			}
			zeile = strcat(zeile, separator);
			
			/* Value */
			zeile = strcat(zeile, maprecord[i].value);
			value_rest = DB_VALLEN - strlen(maprecord[i].value);
			for (s=0; s<value_rest; s++) {
				zeile = strcat(zeile, " ");
			}
			zeile = strcat(zeile, " |\n");
		}
	}
	
	printf("DB List: \n%s \n\n", zeile);
	
	munmap(maprecord, laenge);
	return -42;
}

int main(int argc, char *argv[]) {
	int schreib_fd;
	int map_fd, laenge, i;
	DBRecord test = {"key1  sadas d a", "cat1", "value toll"};
	DBRecord test2 = {"key2", "Kategoriename", "Values sind wirklich super!"};
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
	
	for (i=0; i<(laenge/sizeof(DBRecord)); i++) {
		printf("Key: %s, Cat: %s, Value: %s\n", maprecord[i].key, maprecord[i].cat, maprecord[i].value);
	}
	munmap(maprecord, laenge);
	/* ENDE Einlesen per mmap() ENDE) */
	
	/* In Datenbank nach Eintrag suchen */
	db_search("hier", 0, match);

	/* DB-Eintrag zu Index ausgeben */
	db_get("hier", 0, &index_result); 
	
	/* DB-List ausgeben */
	db_list("hier", schreib_fd, match_filter,"cat1");
		
	
	return 0;
}

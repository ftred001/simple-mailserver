#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> /* Folie 73 */
#include <unistd.h> /* Folie 77 */
#include <sys/types.h> /* Folie 80 */
#include <sys/mman.h> /*Folie 86 */
#include <assert.h>
#include "database.h"


int speichern(int fd, DBRecord *dbr) {
	if (write(fd, dbr, sizeof(DBRecord)) < 0 ) {
		perror("speichern (write)\n");
		return -1;
	}
	return 0;
}

int match_filter(DBRecord *rec, const void *data) {
	if (strlen(data) == 0) {
		return 1;
	}
	
	if (strstr(rec->key, data)) {
		return 1;
	}
	if (strstr(rec->cat, data)) {
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
            close(map_fd);
			return i;
		}
		if (!strcmp(record->cat, maprecord[i].cat)) {
			close(map_fd);
            return i;
		}
	}
	munmap(maprecord, laenge);
    close(map_fd);
	return -1;
}

int db_get(const char *filepath, int index, DBRecord *result) {
	/* returns 0 if successful. else -1 */
	int fd, laenge;
	DBRecord *maprecord;
	
	fd = open(filepath, O_RDWR, 0644);
	laenge = lseek(fd, 0, SEEK_END);
	
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	strcpy(result->key, maprecord[index].key);
	strcpy(result->cat, maprecord[index].cat);
	strcpy(result->value, maprecord[index].value);
    
    close(fd);
	
	printf("=====GET Result=====\n %s %s %s\n\n", result->key, result->cat, result->value);
    
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
	zeile  = strcpy(zeile,"");
	
	in_fd = open(path, O_RDONLY);
	laenge = lseek(in_fd,0, SEEK_END);
	
	data_count = laenge / sizeof(DBRecord);
	
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
	
	printf("==========DB List:==========\n%s \n", zeile);
	
	munmap(maprecord, laenge);
    close(in_fd);
    
	return -42;
}

int db_update(const char *filepath, const DBRecord *new) {
    /* fügt in die Datenbank filepath einen neuen Satz new
 hinzu. Existiert bereits ein Satz mit den gleichen Werten für 
key und cat, so wird dessen value aktualisiert (der Satz wird 
überschrieben und behält seine Index-Nummer). Existiert die gegebene 
key/cat-Kombination noch nicht in der Datenbank, so wird ein entsprechender neuer Satz hinten 
angehängt. Rückgabewert ist in beiden Fällen die Index-Nummer des aktualisierten bzw. 
hinzugefügten Satzes bzw. -1 im Fehlerfall. */

    
	return -42;
}

int db_put(const char *filepath, int index, const DBRecord *record) {
	int fd, laenge;
	DBRecord *maprecord;
    unsigned long data_count;
	
	printf("========PUT OPERATION========\n PUT RECORD %s in Index: %d \n\n", record->key, index);
	
	fd = open(filepath, O_RDWR, 0644);
	laenge = lseek(fd, 0, SEEK_END);
	
    data_count = laenge/sizeof(DBRecord);
    
    
    if (index >= data_count || index < 0) {
        printf("Index-Error DataCount: %ld Geforderter Index: %d\n", data_count, index);
        write(fd, record, sizeof(DBRecord));
        close(fd);
        return 1;
    }
    
	maprecord = mmap(0,laenge, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	strcpy(maprecord[index].key, record->key);
	strcpy(maprecord[index].cat, record->cat);
	strcpy(maprecord[index].value,record->value);
	
	munmap(maprecord, laenge);
	close(fd);
	return 1;
}

int db_del(const char *filepath, int index) {

return -42;
}

int main(int argc, char *argv[]) {
	int schreib_fd, search_result;
	char *filepath = "hier";
	DBRecord test = {"key1", "cat1", "value toll"};
	DBRecord test2 = {"key2", "Kategorie 2", "Values sind wirklich super!"};
	DBRecord test3= {"key3", "cat3", "value toll"};
	DBRecord test4 = {"key4", "cat4", "Values sind wirklich super!"};
	DBRecord such_record = {"key4", "Kategoriename", "Toller Value"};
	DBRecord replace_record = {"key_ersatz", "Kategorieersatz", "Neuer Wert"};
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
	speichern(schreib_fd, &test3);
	speichern(schreib_fd, &test4);
	close(schreib_fd);
	/* ENDE Schreibe Testdaten in Datei ENDE*/
	
	
	search_result = db_search(filepath, 0, &such_record);
	printf("=====SEARCH RESULT=====\nIndex: %d\n==========\n\n", search_result);
	db_get(filepath, 1, &index_result);
	db_list(filepath, schreib_fd, match_filter,"");
	

	db_put(filepath, -1, &replace_record);
	db_list(filepath, schreib_fd, match_filter,"");
	
	return 0;
}

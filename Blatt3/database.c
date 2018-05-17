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
		return 2;
	}
	
	if (strstr(rec->key, data)) {
		return 1;
	}
	if (strstr(rec->cat, data)) {
		return 1;
	}
	return 0;
}

int db_list(const char *path, int outfd, 
  int (*filter)(DBRecord *rec, const void *data), const void *data) {
	/* TODO: Fehlerfälle einarbeiten. */
	int in_fd, file_length,i,s;
	int key_rest, cat_rest, value_rest;
	unsigned long data_count;
	DBRecord *record_map;
	char *zeile = malloc(sizeof(DBRecord)+ 8);
	char *separator = "|";
	int filter_res;
	zeile  = strcpy(zeile,"");
	
	in_fd = open(path, O_RDONLY);
	file_length = lseek(in_fd,0, SEEK_END);
	
	data_count = file_length / sizeof(DBRecord);
	
	record_map = mmap(0, file_length, PROT_READ, MAP_SHARED, in_fd, 0);
		
	
	/* Format String List */
	for (i=0; i<data_count; i++) {
		/* Wenn Filter-Regeln stimmen, dann mache folgendes: */
		if ((filter_res = filter(&record_map[i],data))) {
			zeile = strcat(zeile, separator);
			/* Key */
			zeile = strcat(zeile, record_map[i].key);
			key_rest = DB_KEYLEN - strlen(record_map[i].key);
			for (s=0; s<key_rest; s++) {
				zeile = strcat(zeile, " ");
			}
			zeile = strcat(zeile, separator);
			
			/* Cat */
			zeile = strcat(zeile, record_map[i].cat);
			cat_rest = DB_CATLEN - strlen(record_map[i].cat);
			for (s=0; s<cat_rest; s++) {
				zeile = strcat(zeile, " ");
			}
			zeile = strcat(zeile, separator);
			
			/* Value */
			zeile = strcat(zeile, record_map[i].value);
			value_rest = DB_VALLEN - strlen(record_map[i].value);
			for (s=0; s<value_rest; s++) {
				zeile = strcat(zeile, "");
			}
			zeile = strcat(zeile, separator);
			zeile = strcat(zeile, "\n");
		}
	}
	
	if (filter_res != 2) {
		printf("==========DB List==========\n-------Filter: Ja-------\n%s \n", zeile);
	} else {
		printf("==========DB List==========\n-------Filter: Nein-------\n%s \n", zeile);
	}
	
	
	munmap(record_map, file_length);
    close(in_fd);
    
	return -1;
}


int db_search(const char *filepath, int start, DBRecord *record) {
	/* TODO: Fehlerfall einarbeiten -> return -42 */
	int map_fd, file_length, i;
	unsigned long data_count;
	DBRecord *record_map;
	
	map_fd = open(filepath, O_RDWR, 0644);
	file_length = lseek(map_fd, 0, SEEK_END);
	
	data_count = file_length / sizeof(DBRecord);
	
	record_map = mmap(0,file_length, PROT_READ|PROT_WRITE, MAP_SHARED, map_fd, 0);
	printf("=====SEARCH RESULT Key: %s OR Cat: %s=====\n", record->key, record->cat);

	for (i=0; i<data_count; i++) {
		if (!strcmp(record->key, record_map[i].key)) {
			printf("Key-Match in Index: %d\n\n",i);
			munmap(record_map, file_length);
			close(map_fd);
			return i;
		}
		if (!strcmp(record->cat, record_map[i].cat)) {
			printf("Cat-Match in Index: %d\n\n",i);
			munmap(record_map, file_length);
			close(map_fd);
			return i;
		}
	}
	printf("Error: No Match for Key OR Cat\n\n");
	munmap(record_map, file_length);
    close(map_fd);
	return -1;
}

int db_get(const char *filepath, int index, DBRecord *result) {
	int fd, file_length;
	unsigned long data_count;
	DBRecord *record_map;
	
	fd = open(filepath, O_RDWR, 0644);
	file_length = lseek(fd, 0, SEEK_END);
	data_count = file_length / sizeof(DBRecord);
	
	/* Index > als Anzahl Datensätze */
	if (index>=data_count) {
		close(fd);
		printf("=====GET Result Index: %d=====\nERROR: Index out of Bounds\n\n", index);
		return -1;
	}
	
	/* Kopiere Werte in den Result-Pointer */
	record_map = mmap(0,file_length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	strcpy(result->key, record_map[index].key);
	strcpy(result->cat, record_map[index].cat);
	strcpy(result->value, record_map[index].value);
	
	munmap(record_map, file_length);
    close(fd);
	
	printf("=====GET Result Index: %d=====\n%s %s %s\n\n", index, result->key, result->cat, result->value);
	return 0;
}

int db_put(const char *filepath, int index, const DBRecord *record) {
	int fd, file_length;
	DBRecord *record_map;
    unsigned long data_count;
	
	printf("========PUT OPERATION========\nPUT RECORD %s in Index: %d \n\n", record->key, index);
	
	fd = open(filepath, O_RDWR, 0644);
	file_length = lseek(fd, 0, SEEK_END);
	
    data_count = file_length/sizeof(DBRecord);
    
    
    if (index >= data_count || index < 0) {
        printf("Index-Error DataCount: %ld Geforderter Index: %d\n", data_count, index);
        write(fd, record, sizeof(DBRecord));
        close(fd);
        return 1;
    }
    
	record_map = mmap(0,file_length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	strcpy(record_map[index].key, record->key);
	strcpy(record_map[index].cat, record->cat);
	strcpy(record_map[index].value,record->value);
	
	munmap(record_map, file_length);
	close(fd);
	return 1;
}

int db_update(const char *filepath, const DBRecord *new) {
	int search_result, index=-1;
	DBRecord *copy = calloc(0,sizeof(DBRecord));
	
	strcpy(copy->key, new->key);
	strcpy(copy->cat, new->cat);
	strcpy(copy->value, new->value);
	
	
    search_result = db_search(filepath, 0, copy);
    
    if (search_result != 42) {
		db_put(filepath, search_result, copy);
			
	}
        
	return index;
}



int db_del(const char *filepath, int index) {
	int i, fd, file_length, cache_fd;
	unsigned long data_count;
	char *cachepath = "cachedatei";

	DBRecord *record_map;
	DBRecord *copy_rec = calloc(0, sizeof(DBRecord));

	
	printf("========DELETE OPERATION========\nDELETE RECORD at Index: %d \n\n", index);
	
	fd = open(filepath, O_RDWR, 0644);
	file_length = lseek(fd, 0, SEEK_END);
    data_count = file_length/sizeof(DBRecord);
    
    
    if (index >= data_count || index < 0) {
        printf("Index-Error DataCount: %ld Geforderter Index: %d\n", data_count, index);
        close(fd);
        return -1;
    }
    
    /* Lege Cache-Datei an */
    cache_fd = open(cachepath, O_WRONLY|O_TRUNC|O_CREAT,0644);
    
    /* Kopiere alle Records außer Index */
	record_map = mmap(0,file_length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	for (i=0; i<data_count; i++) {
		if (index!=i) {
			strcpy(copy_rec->key, record_map[i].key);
			strcpy(copy_rec->cat, record_map[i].cat);
			strcpy(copy_rec->value, record_map[i].value);
			if (write(cache_fd, copy_rec, sizeof(DBRecord)) < 0 ) {
				perror("speichern (write)\n");
				return -1;
			}
		}

	}
	munmap(record_map, file_length);
	close(fd);


	/* Benenne Cache-Datei in vorherigen Namen um */
	close(cache_fd);
	rename(cachepath,filepath);
	

	return 1;
}

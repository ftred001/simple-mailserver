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

int db_list(const char *path, int outfd, 
  int (*filter)(DBRecord *rec, const void *data), const void *data) {
	/* TODO: Fehlerfälle prüfen. */
	int infd, file_length,i;
	unsigned long data_count;
	DBRecord *record_map;
	char *zeile = (char*)calloc(1024, sizeof(char));
	int filter_res = 2;
	
	
    if (strlen(path)==0) {
        printf("ERROR: strlen(path) == 0!\n");
        free(zeile);
        return -42;
    }
    
	infd = open(path, O_RDONLY);
    
    if (infd < 0) {
        perror("Bei Oeffnen der Eingabedatei");
        free(zeile);
        return -42;
    }
    
	file_length = lseek(infd,0, SEEK_END);
    
    if (file_length < 0) {
        perror("Beim Lesen der Eingabedatei");
        close(infd);
        free(zeile);
        return -42;
    }
    
    if (outfd < 1) {
        perror("Schreibdatei ungültig: <1");
        return -42;
    }
	
	data_count = file_length / sizeof(DBRecord);	
	record_map = mmap(0, file_length, PROT_READ, MAP_SHARED, infd, 0);
		
	
	
	
	/* Format String List */
	for (i=0; i<data_count; i++) {
		/* Wenn Filter-Regeln stimmen, dann mache folgendes: */
		if ((filter_res = filter(&record_map[i],data))) {
			
			sprintf(zeile, "%d: %s | %s | %s\n", i, record_map[i].key, record_map[i].cat, record_map[i].value);
			        

            if (write(outfd, zeile, strlen(zeile)) <0) {
                perror("speichern (write)");
            }
		}

	}
	

	
	munmap(record_map, file_length);
    close(infd);
    
	return data_count;
}

/* Sucht Inhalt ab Offset und gibt Index zurück. */
int db_search(const char *filepath, int start, DBRecord *record) {
	/* TODO: Fehlerfall einarbeiten -> return -42 */
	int map_fd, file_length, i;
	unsigned long data_count;
	DBRecord *record_map;
	
	/*
	printf("---DB_SEARCH---\n");
	*/
    
    if (record == NULL) {
        perror("DBRecord ist NULL");
        return -42;
    }
    
    if (start < 0) {
        perror("INDEX ERROR: int<0!");
        return -42;
    }
    
    if (strlen(filepath)<1) {
        perror("Filepath Error! strlen(filepath)<1!");
        return -42;
    }
    
	map_fd = open(filepath, O_RDONLY, 0644);
    
    if (map_fd<0) {
        perror("Filedeskriptor Error! <0");
        return -42;
    }
    
	file_length = lseek(map_fd, 0, SEEK_END);
    
    if (file_length <0) {
        perror("Filelength Error! <0\n");
        return -42;
    }
	
	data_count = file_length / sizeof(DBRecord);
	
	record_map = mmap(0,file_length, PROT_READ, MAP_SHARED, map_fd, 0);
    /* printf("Key: %s - Cat: %s\n", record->key, record->cat); */
    
    /* Durchsuche Datenbank */
    for (i=0; i<data_count; i++) {
        
        /* Suche Match von key UND cat */
        if (strlen(record->key) && strlen(record->cat)) {
            if ((!strcmp(record->key, record_map[i].key)) && (!strcmp(record->cat, record_map[i].cat))) {
                strcpy(record->value, record_map[i].value);
                munmap(record_map, file_length);
                close(map_fd);
                return i;
            }
        } else if (strlen(record->key) && !strlen(record->cat)) {
            /* Suche wenn nur Key gesetzt ist. */
            if (!strcmp(record->key, record_map[i].key)) {
                strcpy(record->value, record_map[i].value);
                munmap(record_map, file_length);
                close(map_fd);
                return i;
            }
		} else if(strlen(record->cat) && !strlen(record->key)) {
            /* Suche nur nach Cat */
            if (!strcmp(record->cat, record_map[i].cat)) {
                strcpy(record->value, record_map[i].value);
                munmap(record_map, file_length);
                close(map_fd);
                return i;
            }
        }
    }
    
	munmap(record_map, file_length);
    close(map_fd);
	return -1;
}

/* Schreibt Inhalte in result-Pointer */
int db_get(const char *filepath, int index, DBRecord *result) {
DBRecord buffer;
	int fd, gelesen;
	fd = open(filepath, O_RDONLY);
	
	if(lseek(fd,sizeof(DBRecord)*index,SEEK_SET) < 0) {
		perror("Error");
		close(fd);
		return -1;
	}
	
	gelesen = read(fd, &(buffer), sizeof(DBRecord));
		
	if(gelesen < 0){
		perror("Lesefehler");
		close(fd);
		return -1;
	}
	
	*result = buffer;
	
	close(fd);
	return 0;
}

/* Schreibt Datensatz an Indexstelle oder Ende. */
int db_put(const char *filepath, int index, const DBRecord *record) {
	int fd, file_length;
	DBRecord *record_map;
    unsigned long data_count;
	
	printf("========PUT OPERATION========\n\n");
	
	fd = open(filepath, O_RDWR, 0644);
	file_length = lseek(fd, 0, SEEK_END);
	
    data_count = file_length/sizeof(DBRecord);
    
    
    if (index >= data_count || index < 0) {
        printf("CREATE NEW RECORD (Index: %lu)\n\n", data_count);
        write(fd, record, sizeof(DBRecord));
        close(fd);
        return 1;
    }
    
	record_map = mmap(0,file_length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	strcpy(record_map[index].key, record->key);
	strcpy(record_map[index].cat, record->cat);
	strcpy(record_map[index].value,record->value);
    
    printf("UPDATE RECORD (Index: %d)", index);
	
	munmap(record_map, file_length);
	close(fd);
	return 1;
}

/* Aktualisiert Value von gefundenem Datensatz oder fügt Datensatz ein. */
int db_update(const char *filepath, const DBRecord *new) {
	int search_result;
    
    printf("======UPDATE======\n");
	
    search_result = db_search(filepath, 0, (DBRecord*) new);
    
    if (search_result != -42) {
		printf("Datensatz gefunden bei %d \n",search_result);
		db_put(filepath, search_result, new);
	} else {
		return -1;
	}
        
	return 0;
}



int db_del(const char *filepath, int index) {
	int i, fd, file_length, cache_fd;
	unsigned long data_count;
	char *cachepath = "cachedatei";

	DBRecord *record_map;
	DBRecord *copy_rec = calloc(1, sizeof(DBRecord));

	
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

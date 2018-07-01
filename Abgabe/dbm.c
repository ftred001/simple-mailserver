#include <stdio.h>
#include <stdlib.h>
#include <limits.h> /* strtoul */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "database.h"
#include "dialog.h"
#include "dbm.h"


int speichern(int fd, DBRecord *dbr);
int match_filter(DBRecord *rec, const void *data);


int key_filter(DBRecord *rec, const void *data) {
    if (strstr(rec->key, data)) {
		return 1;
	}
    return 0;
}

int cat_filter(DBRecord *rec, const void *data) {
    if (strstr(rec->cat, data)) {
		return 1;
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

int execute_cmd(const char *filepath, int argc, char *line) {
	int fd;
	int outfd = 1;
	int i=0;
	long index;
	DBRecord rec;
	char *errstring;
	char delimiter[] = " ";
	char *argv[10]; /* Maximale Anzahl an Parametern. */
	char *wort;
	
	line[strlen(line)-1] = '\0';
	
	wort = strtok(line, delimiter);
	for (i=0; i<=argc; i++) {
		if (wort != NULL) {
			argv[i] = calloc(1, LINEMAX);
			strcpy(argv[i], wort);
			wort = strtok(NULL, delimiter);	
		}
	}
	
	
	/* Erstellt Datei, falls nicht existiert */
	if (strlen(filepath)) {
		fd = open(filepath, O_RDONLY|O_CREAT,0644);
	} else {
		fd = open(STD_FILEPATH, O_RDONLY|O_CREAT,0644);
	}
	
	close(fd);

	
	if (argc == 1) {
		if (!strcmp(argv[0], "list")) {	
			printf("===LIST ALL===\n");
			db_list(filepath, outfd, match_filter,""); 
		}
	}
	
	if (argc == 2) {
        /* List mit Key Filter */
		if (!strcmp(argv[0], "list")) {
			printf("===LIST===\n");
            db_list(filepath, outfd, key_filter,argv[1]); 
        }
        
        /* List mit Catfilter */
		if (!strcmp(argv[0], "clist")) {
			printf("===CLIST===\n");
            db_list(filepath, outfd, cat_filter,argv[1]); 
        }
        
        /* Suche: Key oder Cat */
        /* "search keyword" */
		if (!strcmp(argv[0], "search")) {
			printf("===SEARCH KEY || CAT ===\n");
			strcpy(rec.key,argv[1]);
			strcpy(rec.cat,argv[1]);
			printf("Search Result:%d\n", db_search(filepath, 0, &rec));
		}
        
		/* Lösche Indexnummer */
		/* "delete indexnummer" */
        if (!strcmp(argv[0], "delete")) {
			index = strtoul(argv[1], &errstring, 10);
            db_del(filepath, index);
		}
	}
	
	if (argc == 3) {
		/* Suche: Key UND Cat */
        /* "search keyword" */
		if (!strcmp(argv[0], "search")) {
			printf("===SEARCH Key && CAT===\n");
			strcpy(rec.key,argv[1]);
			strcpy(rec.cat,argv[2]);
			printf("Search Result:%d\n", db_search(filepath, 0, &rec));
		}
	
	}
		
	if (argc == 4) {
		
		if (!strcmp(argv[0], "update")) {
			strcpy(rec.key,argv[1]);
			strcpy(rec.value,argv[2]);
			strcpy(rec.value, argv[3]);
			db_update(filepath, &rec);
		}
		
		if (!strcmp(argv[0], "add")) {
			strcpy(rec.key,argv[1]);
			strcpy(rec.cat,argv[2]);
			strcpy(rec.value,argv[3]);
			db_put(filepath, -1, &rec);
		}
	}
	
	return 0;
}


int main(int argc, char *argv[]) {
	char *line = calloc(LINEMAX, sizeof(char));
	char *mem = calloc(LINEMAX, sizeof(char));
	int linecounter;
	char *wort;
	char delimiter[] = " ";
	

	
	while (fgets(line, LINEMAX, stdin) != NULL) {
		strcpy(mem, line);
		linecounter = 0;

		wort = strtok(line, delimiter);
	
		while(wort != NULL) {
			linecounter++;
			wort = strtok(NULL, delimiter);
		}
		execute_cmd(STD_FILEPATH, linecounter, mem);
	}
	
	printf("=======\nDataBaseManager QUIT======\n");
	
	
	return 0;
}

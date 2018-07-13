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


int main(int argc, char *argv[]) {
	int fd;
	int outfd = 1;
	long index;
	DBRecord rec = {0};
	char *errstring;
	const char *filepath = "mailserver.db";
		
	
	/* Erstellt Datei, falls nicht existiert */
	if (strlen(filepath)) {
		fd = open(filepath, O_RDONLY|O_CREAT,0644);
	} else {
		fd = open(STD_FILEPATH, O_RDONLY|O_CREAT,0644);
	}
	
	close(fd);
	
	printf("%d %s %s\n", argc, argv[0], argv[1]);

	
	if (argc == 2) {
		if (!strcmp(argv[1], "list")) {	
			printf("===LIST ALL===\n");
			db_list(filepath, outfd, match_filter,""); 
		}
	}
	
	if (argc == 3) {
        /* List mit Key Filter */
		if (!strcmp(argv[1], "list")) {
			printf("===LIST===\n");
            db_list(filepath, outfd, key_filter,argv[1]); 
        }
        
        /* List mit Catfilter */
		if (!strcmp(argv[1], "clist")) {
			printf("===CLIST===\n");
            db_list(filepath, outfd, cat_filter,argv[1]); 
        }
        
        /* Suche: Key oder Cat */
        /* "search keyword" */
		if (!strcmp(argv[1], "search")) {
			printf("===SEARCH KEY || CAT %s ===\n", argv[2]);
			strcpy(rec.key,argv[2]);
			strcpy(rec.cat,argv[2]);
			printf("Search Result:%d\n", db_search(filepath, 0, &rec));
		}
        
		/* Lösche Indexnummer */
		/* "delete indexnummer" */
        if (!strcmp(argv[1], "delete")) {
			index = strtoul(argv[2], &errstring, 10);
            db_del(filepath, index);
		}
	}
	
	if (argc == 4) {
		/* Suche: Key UND Cat */
        /* "search keyword" */
		if (!strcmp(argv[1], "search")) {
			printf("===SEARCH Key && CAT===\n");
			strcpy(rec.key,argv[2]);
			strcpy(rec.cat,argv[3]);
			printf("Search Result:%d\n", db_search(filepath, 0, &rec));
		}
	
	}
		
	if (argc == 5) {
		
		if (!strcmp(argv[1], "update")) {
			strcpy(rec.key,argv[2]);
			strcpy(rec.value,argv[3]);
			strcpy(rec.value, argv[4]);
			db_update(filepath, &rec);
		}
		
		if (!strcmp(argv[1], "add")) {
			strcpy(rec.key,argv[2]);
			strcpy(rec.cat,argv[3]);
			strcpy(rec.value,argv[4]);
			
			if (db_search(filepath, 0, &rec) == -1) {
				db_put(filepath, -1, &rec);
			} else {
				printf("Already exists. Did you mean update?\n");
			}
		}
	}
	
	return 0;
}

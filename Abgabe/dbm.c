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
	
	DBRecord rec1 = {"joendhard","mailbox","joendhard.mbox"};
	DBRecord rec2 = {"joendhard","password","biffel"};
	DBRecord rec3 = {"host","pop3","127.0.0.1"};
	DBRecord rec4 = {"port","pop3","8110"};
	DBRecord rec5 = {"host","smtp","127.0.0.1"};
	DBRecord rec6 = {"port","smtp","8025"};
	DBRecord rec7 = {"j.biffel@mymaildings.de","smtp","joendhard"};

		
	
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
		
		
		if (!strcmp(argv[1], "test")) {
			printf("Generating Testdata...\n");
			if (db_search(filepath, 0, &rec1) == -1) db_put(filepath, -1, &rec1);
			if (db_search(filepath, 0, &rec2) == -1) db_put(filepath, -1, &rec2);
			if (db_search(filepath, 0, &rec3) == -1) db_put(filepath, -1, &rec3);
			if (db_search(filepath, 0, &rec4) == -1) db_put(filepath, -1, &rec4);
			if (db_search(filepath, 0, &rec5) == -1) db_put(filepath, -1, &rec5);
			if (db_search(filepath, 0, &rec6) == -1) db_put(filepath, -1, &rec6);
			if (db_search(filepath, 0, &rec7) == -1) db_put(filepath, -1, &rec7);

			printf("Creating Testdata completed\n");
		}
		
	}
	
	if (argc == 3) {
        /* List mit Key Filter */
		if (!strcmp(argv[1], "list")) {
			printf("===LIST===\n");
            db_list(filepath, outfd, key_filter,argv[2]); 
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
		
		/* Add mit StandardValue */
		if (!strcmp(argv[1], "add")) {
			strcpy(rec.key,argv[2]);
			strcpy(rec.cat,argv[3]);
			strcpy(rec.value, "---");
			if (db_search(filepath, 0, &rec) == -1) {
				db_put(filepath, -1, &rec);
			} else {
				printf("Already exists. Did you mean update?\n");
			}
		}
		
	
	}
		
	if (argc == 5) {
		if (!strcmp(argv[1], "update")) {
			strcpy(rec.key,argv[2]);
			strcpy(rec.cat,argv[3]);
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

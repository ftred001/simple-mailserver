#include <stdio.h>
#include <stdlib.h>
#include <limits.h> /* strtoul */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "database.h"
#include "dialog.h"


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

void create_testdata(char *filepath) {
	int fd;
	/* Schreibe Testdaten in Datei */
	DBRecord test = {"key1", "cat1", "value toll"};
	DBRecord test2 = {"key2", "Kategorie 2", "Values sind wirklich super!"};
	DBRecord test3= {"key3", "cat3", "value toll"};
	DBRecord test4 = {"key4", "cat4", "Values sind wirklich super!"};
	

	fd = open(filepath, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	if (fd < 0) {
		perror("Bei Oeffnen der Ausgabedatei");
		exit(3);
	}
	speichern(fd, &test);
	speichern(fd, &test2);
	speichern(fd, &test3);
	speichern(fd, &test4);
	close(fd);
}

int test(char *filepath) {
	int search_result;
	DBRecord such_treffer = {"key4", "Kategoriename", "Toller Value"};
	DBRecord such_niete = {"Schlüssel", "Wursti", "Toller Value"};
	DBRecord replace_record = {"key_ersatz", "Kategorieersatz", "Neuer Wert"};
	const DBRecord update_record = {"key1", "cat1", "Dieser Value ist noch viel toller als der vorherige"};
	DBRecord index_result;
	int outfd;
	
	outfd = open(filepath, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	
	db_get(filepath, 17, &index_result);
	
	search_result = db_search(filepath, 0, &such_treffer);
	search_result = db_search(filepath, 0, &such_niete);
	
	db_get(filepath, search_result, &index_result);

	db_list(filepath, outfd, match_filter,"key");
	

	db_put(filepath, -1, &replace_record);
	db_list(filepath, outfd, match_filter,"");
	
	db_update(filepath, &update_record);
	db_list(filepath, outfd, match_filter, "");
	
	db_del(filepath, 5);
	db_list(filepath, outfd, match_filter, "");
	return 1;
}

int execute_cmd(const char *filepath, int argc, char *line) {
	int fd;
	int outfd = 1;
	int i=0;
	long index;
	DBRecord rec;
	char *errstring;
	char delimiter[] = " ";
	char *argv[10];
	char *wort;
	
	wort = strtok(line, delimiter);
	for (i=0; i<=argc; i++) {
		if (wort != NULL) {
			argv[i] = calloc(1, LINEMAX);
			strcpy(argv[i], wort);
			if (i==argc-1) {
				/* Cutte letztes \n der Eingabe */
				argv[i][strlen(wort)-1] = '\0';
			}
			wort = strtok(NULL, delimiter);	
		}
	}
	
	
	/* Testausgabe von argv */
	for(i=0;i<argc;i++) {
		printf("%d | Len: %ld  String: %sX\n", i, strlen(argv[i]), argv[i]);
	}
	
	
	/* Erstellt Datei, falls nicht existiert */
	fd = open(filepath, O_RDONLY|O_CREAT,0644);
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
			printf("===SEARCH===\n");
			strcpy(rec.key,argv[1]);
			strcpy(rec.cat,argv[1]);
			db_list(filepath, outfd, match_filter, &rec);
		}
        
		/* Lösche Indexnummer */
		/* "delete indexnummer" */
        if (!strcmp(argv[0], "delete")) {
			index = strtoul(argv[1], &errstring, 10);
            db_del(filepath, index);
		}
	}
	
	if (argc == 3) {
       
        if (!strcmp(argv[0], "update")) {
			strcpy(rec.key,argv[1]);
			strcpy(rec.value,argv[2]);
			db_update(filepath, &rec);
		}
        
	}
	
	if (argc == 4) {
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
	const char *filepath = "serversettings.cfg";
	char *line = calloc(LINEMAX, sizeof(char));
	char *mem = calloc(LINEMAX, sizeof(char));
	int linecounter;
	char *wort;
	char delimiter[] = " ";
	

	printf("=========\nWillkommen im DataBaseManager Bitte nutzen Sie tolle Commands!\n===========\n\n");
	
	/*  Einlesen bis EOF */
	while (fgets(line, LINEMAX, stdin) != NULL) {
		strcpy(mem, line);
		linecounter = 0;
		/* Argumente zählen */
		wort = strtok(line, delimiter);
	
		while(wort != NULL) {
			linecounter++;
			printf("%d %s\n", linecounter, wort);
			wort = strtok(NULL, delimiter);
		}
		execute_cmd(filepath, linecounter, mem);
	}
	
	printf("=======\nDataBaseManager QUIT======\n");
	
	
	return 0;
}

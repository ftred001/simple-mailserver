#include <stdio.h>
#include <stdlib.h>

#include "database.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int speichern(int fd, DBRecord *dbr);
int match_filter(DBRecord *rec, const void *data);

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
	int schreib_fd;
	
	schreib_fd = open(filepath, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	
	db_get(filepath, 17, &index_result);
	
	search_result = db_search(filepath, 0, &such_treffer);
	search_result = db_search(filepath, 0, &such_niete);
	
	db_get(filepath, search_result, &index_result);

	db_list(filepath, schreib_fd, match_filter,"key");
	

	db_put(filepath, -1, &replace_record);
	db_list(filepath, schreib_fd, match_filter,"");
	
	db_update(filepath, &update_record);
	db_list(filepath, schreib_fd, match_filter, "");
	
	db_del(filepath, 5);
	db_list(filepath, schreib_fd, match_filter, "");
	return 1;
}

int main(int argc, char *argv[]) {
	char *filepath;
	
	if (argc!=3) { printf("Falscher Aufruf\n"); exit(1); }
	
	filepath = argv[2];
	
	create_testdata(filepath);
	test(filepath);
	
	return 0;
}

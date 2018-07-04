#include "fileindex.h"
#include "linebuffer.h"

#include <fcntl.h> /* open() */
#include <unistd.h> /* read, write, close */
#include <sys/types.h> /* lseek */
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>




/* Ziel:
 * Betrachtete Textdatei in indexierbare Abschnitte einteilen.
 * 
 * Jeder Abschnitt enthält eine SEPARATORZEILE, die NICHT zum Inhalt gehört.
 * ACHTUNG: Leerzeilen gehören zum Inhalt.
 *  */
 
 void print_entry(FileIndexEntry *e) {
	
	if (e == NULL) {
		printf("ENTRY IS NULL!!!\n");
		return;
	} 
	
	 
	printf("----Entry #%d----\n", e->nr);
	printf("Seekpos: %d\n", e->seekpos);
	printf("Size: %d\n", e->size);
	printf("Lines: %d\n", e->lines);
	printf("DEL_FLAG %d\n", e->del_flag);
	if (e->next == NULL) {
		printf("NEXT -> NULL\n");
	} else {
		printf("NEXT -> NOT NULL\n");
	}
	printf("\n\n");
}

void print_fi(FileIndex *fi) {
	if (fi == NULL) {
		printf("CANNOT PRINT fi = NULL\n");
		return;
	}
	
	printf("----FileIndex----\n");
	printf("Filepath: %s\n", fi->filepath);
	printf("nEntries: %d\n", fi->nEntries);
	printf("totalSize: %d\n", fi->totalSize);
	printf("\n\n");
}
 
 void print_entries(FileIndex *fi) {
	FileIndexEntry *e = fi->entries;
	printf(">>>Print Entries\n");
	
	
	if (e == NULL) {
		printf("---NO ENTRIES FOUND!\n");
		return;
	}
	
	while(e != NULL) {
		print_entry(e);
		e = e->next;
	}
}

 
/* Indiziert angegebene Datei filepath. Trennzeilen sind wie SEPARATOR und gehören NICHT zum Inhalt
 * return Zeiger auf FileIndex wenn okay
 * return NULL bei Fehler
 * */
FileIndex *fi_new(const char *filepath, const char *separator) {
	FileIndex *findex = NULL;
	FileIndexEntry *entry;
	FileIndexEntry *ptr;
	char *line = (char*)calloc(LINEBUFFERSIZE, sizeof(char));
	LineBuffer *b = NULL;
	int fd, umbruch=0;
	int linestart;
	/* int lineend; */
	
	if ((findex = (FileIndex*)calloc(1, sizeof(FileIndex))) == NULL) {
		perror("Beim allokieren des Speichers für FileIndex");
		return NULL;
	}
	
	findex->filepath = filepath;
	
	
	if ((fd=open(filepath, O_RDONLY, 0644))<0) {
		perror("Beim Öffnen des Filepaths");
	}
	
	if ((b = buf_new(fd, separator))== NULL) {
		perror("buf_new == NULL");
	}
	
	
	
	/* FIEntry für jeden Abschnitt. */
	while ((umbruch = buf_readline(b, line, LINEBUFFERSIZE)) !=-1) {
		findex->totalSize =  buf_where(b); 
		linestart = findex->totalSize - strlen(line)- b->lineseplen;
		
		/*
		lineend = findex->totalSize;
		printf("buf_where: %d strlen: %ld lineseplen: %d\n", buf_where(b), strlen(line), b->lineseplen);
        printf("Start: %d End:%d line: %s \n", linestart, lineend,line);
		*/
		
		/* Sektionsanfang */
		if (!strncmp(line, "From ", 5)) {
			/*printf("---SECTIONSTART---\n"); */
			
			findex->nEntries++;
			
			entry = calloc(1, sizeof(FileIndexEntry));
			entry->nr = findex->nEntries;
			entry->seekpos = linestart;
			
			if (!findex->entries) {
				findex->entries = entry;
			} else {
				ptr = findex->entries;
				while(ptr->next != NULL) {
					ptr = ptr->next;
				}
				ptr->next = entry;				
			}
		}
		
		if (entry->nr) {
			entry->lines++;
			entry->size += strlen(line)+b->lineseplen;
			
		}
				
		
		/* Wenn Zeile nicht über Buffer hinausgeht */
		if (umbruch != -2) {
			line[0] = '\0';
		}
	} 

	
	buf_dispose(b);
	
	close(fd);
	
	return findex;
	
}

/* Gibt Speicher für FileIndex-Struct frei
 * ACHTUNG: INKLUSIVE Liste der FileIndex Knoten 
 */
void fi_dispose(FileIndex *fi) {
	FileIndexEntry *entry = fi->entries;
	FileIndexEntry *n;
	
	printf(">>>fi_dispose(FileIndex *fi)\n");
	
	while (entry->next != NULL) {
		n = entry->next;
		free(entry);
		entry = NULL;
		entry = n;
	}
	free(entry);
	entry = NULL;
	
	free(fi);
	fi = NULL;
	printf("---Disposing Entries succesful!\n");
}

/* return Zeiger auf FileIndexEntry zum Listenelement n
 * ACHTUNG: Zählung beginnt bei "1" <- EINS
 * return NULL bei Fehler
 * */
FileIndexEntry *fi_find(FileIndex *fi, int n) {
	FileIndexEntry *res = NULL;
	
	printf(">>>Find Entry #%d\n",n);
	
	if (fi == NULL) {
		perror("file_index cannot be NULL!");
		return NULL;
	}
	
	if (n<1) {
		perror("Muss mind. 1 sein"); 
		return NULL;
	}
	
	if (n>fi->nEntries) {
		perror("Out of range");
		return NULL;
	}
	
	res = fi->entries;
	
	while(res != NULL) {
		if (res->nr == n) {
			printf("---Entry #%d found\n",n);
			return res;
		}
		res = res->next;
	}
	
	perror("Entry NOT found\n");
	return NULL;
}


/* Löscht alle Abschnitte, wo del_flag == 1 aus der DATEI!!!!. -> Umkopieren + Umbenennen
 * return 0 bei Erfolg.
 * return != 0 bei FEHLER
 */
int fi_compactify(FileIndex *fi) {
	/* TODO Richtig implementieren!!! */ 
	printf(">>>fi_compactify(FileIndex *fi)\n");
	perror("fi_compactify NICHT implementiert!");
	
	return 0;
}

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

void print_fi(FileIndex *i) {
	printf("----FileIndex----\n");
	printf("Filepath: %s\n", i->filepath);
	printf("nEntries: %d\n", i->nEntries);
	printf("totalSize: %d\n", i->totalSize);
	printf("\n\n");
}
 
 FileIndexEntry *new_entry(int nr) {
	FileIndexEntry *entry = calloc(1, sizeof(FileIndexEntry));
	
	entry->next = NULL;
	entry->nr = nr;
	
	printf("New Entry #%d\n", entry->nr);
	
	return entry;
}

 void print_entries(FileIndex *fi) {
	FileIndexEntry *e = NULL;
	
	e = fi->entries;
	
	while(e->next != NULL) {
		print_entry(e);
		e = e->next;
	}
}

void append_entry(FileIndex *i, FileIndexEntry *e)  {
	FileIndexEntry *ptr;
	
	printf("APPEND ENTRY\n");
	
	if (i->entries == NULL) {
		printf("Entries = NULL\n");
		i->entries = e;
		return;
	}
	
	printf("ENTRIES NOT NULL\n");
	
	ptr = i->entries;
	
	while(ptr->next != NULL) {
		ptr = ptr->next;
	}
	
	ptr->next = e;
	return;
}
 
 

 
/* Indiziert angegebene Datei filepath. Trennzeilen sind wie SEPARATOR und gehören NICHT zum Inhalt
 * return Zeiger auf FileIndex wenn okay
 * return NULL bei Fehler
 * */
FileIndex *fi_new(const char *filepath, const char *separator) {
	FileIndex *findex = calloc(1, sizeof(FileIndex));
	FileIndexEntry *entry, *ptr;
	char *line = calloc(1024, sizeof(char));
	LineBuffer *b;
	int fd, umbruch=0;
	int is_head=0;
	
	
	findex->filepath = filepath;
	
	fd = open(filepath, O_RDONLY, 0644);
	
	b = buf_new(fd, separator);
	
	/* FIEntry für jeden Abschnitt. */
	while ((umbruch = buf_readline(b, line, LINEBUFFERSIZE)) !=-1) {
		/* NUR WENN TEXT GELESEN WURDE */
		if (umbruch >=0) {
			findex->totalSize =  findex->totalSize + umbruch + b->lineseplen; 
			
			/* Sektionsanfang */
			if (!strncmp(line, "From ", 5)) {
				findex->nEntries++;
				is_head = 1;
				
				
				if (!findex->entries) {
					findex->entries = new_entry(findex->nEntries);
					entry = findex->entries;
				} else {
					ptr = findex->entries;
					while(ptr->next != NULL) {
						ptr = ptr->next;
					}
					ptr->next = new_entry(findex->nEntries);
					entry = ptr->next;
									
				}
				
			}
			
			if (entry->nr && is_head == 0) {
				entry->lines++;
				entry->size += strlen(line) + b->lineseplen;
				
			}
			
			/* Erste Leerzeile finden -> HEAD Ende */
			if (!strcmp(line, "") && is_head) {
				entry->seekpos = buf_where(b)+b->lineseplen;
				is_head = 0;
			}
			 
		}
		
		if (umbruch == -2) {
			line = "";
		}
	} 

	
	buf_dispose(b);
	
	close(fd);
	
	return findex;
	
}

/* Gibt Speicher für FileIndex-Struct frei
 * ACHTUNG: INKLUSIVE Liste der FileIndex Knoten 
 */
void fi_dispose(FileIndex *fi);

/* return Zeiger auf FileIndexEntry zum Listenelement n
 * ACHTUNG: Zählung beginnt bei "1" <- EINS
 * return NULL bei Fehler
 * */
FileIndexEntry *fi_find(FileIndex *fi, int n);


/* Löscht alle Abschnitte, wo del_flag == 1 ist. -> Umkopieren + Umbenennen
 * return 0 bei Erfolg.
 * return != 0 bei FEHLER
 */
int fi_compactify(FileIndex *fi);



int main(int argc, char *argv[]) {
	FileIndex *findex;
	const char *line_separator = "\n";

	
	printf("Lese folgende Datei ein: %s\n\n", argv[1]);
	
	findex = fi_new(argv[1], line_separator);

	print_entries(findex);
	
	return 0;
}

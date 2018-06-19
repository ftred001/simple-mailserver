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
 
 FileIndexEntry *new_entry(int nr) {
	FileIndexEntry *entry = calloc(sizeof(FileIndexEntry), 0);
	entry->next = NULL;
	entry->nr = nr;
	return entry;
}

FileIndexEntry *append_entry(FileIndexEntry *parent, FileIndexEntry *child) {
	parent->next = child;
	return child;
}
 
/* Indiziert angegebene Datei filepath. Trennzeilen sind wie SEPARATOR und gehören NICHT zum Inhalt
 * return Zeiger auf FileIndex wenn okay
 * return NULL bei Fehler
 * */
FileIndex *fi_new(const char *filepath, const char *separator) {
	FileIndex *findex = calloc(sizeof(FileIndex), 0);
	FileIndexEntry *entry, *mem;
	char *line = calloc(1024, sizeof(char));
	LineBuffer *b;
	int fd, umbruch=0, offsetpos=0;
	int is_head=0;
	
	
	findex->filepath = filepath;
	
	fd = open(filepath, O_RDONLY, 0644);
	
	b = buf_new(fd, separator);
	
	/* FIEntry für jeden Abschnitt. */
	while ((umbruch = buf_readline(b, line, LINEBUFFERSIZE)) !=-1) {
		 if (umbruch >= 0) {
			findex->totalSize += umbruch + b->lineseplen; 
			
			/* Sektionsanfang */
			if (!strncmp(line, "From ", 5)) {
				findex->nEntries++;
				
				if (!findex->nEntries) {
					printf("INIT HEAD\n");
					entry = new_entry(findex->nEntries);
					findex->entries = entry;
				} else {
					printf("APPEND SECTION\n");
					/* Merke alten Entry */
					/* Erstelle neuen Entry */
					/* Lass alten Entry auf neuen Entry zeigen */
					
					mem = entry;
					entry = new_entry(findex->nEntries);
					append_entry(mem, entry);
				}
				is_head = 1;
				printf("%d ", buf_where(b));
				printf("ENTRY #%d Size: %d Lines %d\n",entry->nr,entry->size,entry->lines);
			}
			
			if (entry->nr && is_head == 0) {
				entry->lines++;
				entry->size += strlen(line) + b->lineseplen;
			}
			
			/* Erste Leerzeile finden -> HEAD Ende */
			if (!strcmp(line, "") && is_head) {
				printf("%d LEER\n", buf_where(b));
				entry->seekpos = buf_where(b)+b->lineseplen;
				is_head = 0;
			}
			 
		}
	} 
	
	printf("ENTRY #%d Size: %d Lines %d\n",entry->nr,entry->size,entry->lines);

	
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
	
	
	
	return 0;
}

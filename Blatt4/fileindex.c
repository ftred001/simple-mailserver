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
 * 
 * 
 * 
 *  */
 
 
/* Indiziert angegebene Datei filepath. Trennzeilen sind wie SEPARATOR und gehören NICHT zum Inhalt
 * return Zeiger auf FileIndex wenn okay
 * return NULL bei Fehler
 * */
FileIndex *fi_new(const char *filepath, const char *separator) {
	FileIndex *findex = calloc(sizeof(FileIndex), 0);
	char *line = calloc(0, LINEBUFFERSIZE);
	LineBuffer *lbuffer;
	int fd;
	
	findex->filepath = filepath;
	
	fd = open(filepath, O_RDONLY, 0644);
	
	lbuffer = buf_new(fd, separator);
	
	/* FIEntry für jeden Abschnitt. */
	while (buf_readline(lbuffer, line, LINEBUFFERSIZE)) {
		 print_pos(lbuffer);
	} 
	
	buf_dispose(lbuffer);
	
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
	const char *line_separator = ".\r\n";

	
	printf("Lese folgende Datei ein: %s\n\n", argv[1]);
	
	findex = fi_new(argv[1], line_separator);
	
	
	
	return 0;
}

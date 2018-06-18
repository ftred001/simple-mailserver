#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> /* open() */
#include <unistd.h> /* read, write, close */
#include <sys/types.h> /* lseek */
#include <errno.h>
#include "linebuffer.h"

/*
 * To Test: buf_dispose(LineBuffer lb) wirklich freigegeben? 
 * To Test: buf_readline() Wird richtige POS returned?
 * To Test: buf_readline() Zwei Zeichen (Zeilenende/-anfang) verarbeiten.
 * To Test: buf_where() Prüfen ob korrekt.
 * To Test: buf_seek() Testen ob richtig verschoben wird.
 */
 


LineBuffer *buf_new(int descriptor, const char *linesep) {
	LineBuffer *lb;
	lb = calloc(0,sizeof(LineBuffer));
	lb->descriptor = descriptor;
	lb->linesep = linesep;
	lb->lineseplen = strlen(linesep);
	return lb;
}

void buf_dispose(LineBuffer *lb) {
	free(lb);
	lb = NULL;
	printf("-----LineBuffer disposed-----\n");
}

void print_buffer(LineBuffer *lb) {
	printf("\n-----LINEBUFFER-----\n");
	printf("bytesread %u | here: %u | end: %u ", lb->bytesread, lb->here, lb->end);
	
	if (!strcmp("\r\n", lb->linesep)) printf("Separator: \\r\\n Length: %d\n", lb->lineseplen);
	if (!strcmp("\n", lb->linesep)) printf("Separator: \\n Length: %d\n", lb->lineseplen);
	if (!strcmp("\r", lb->linesep)) printf("Separator: \\r Length: %d\n", lb->lineseplen);
	
	printf("IM BUFFER: %s\n", lb->buffer);
	printf("---LINEBUFFER END---\n\n");
}

void fill_buffer(LineBuffer *b, char *line, int linemax) {


}

int buf_readline(LineBuffer *b, char *line, int linemax) {
	int hit_index=0;
	
	/* Wenn ich am Anfang oder Ende der LINE bin, lese ich eine neue LINE ein. */
	if (b->here == b->end) {
		b->end = read(b->descriptor, b->buffer, linemax);
		if (b->end == 0) {
			/* Dateiende */
			printf("---END OF FILE---\n\n");
			return -1;
		} else if (b->end <0) {
			perror("Lesefehler\n");
			return -42;
		}	
		b->bytesread += b->end;
		b->here = 0;
	} 
	
	
	/* Suche Zeilenumbruch und gib Zeilenanfang zurück */
	while (b->here < b->end) {
		
		/* Finde Vorkommen des LineSeparators */
		if (b->buffer[b->here] == b->linesep[hit_index]) {
			hit_index++;
			
			/* Wenn Treffer, aber am ENDE der Zeile: Lese mehr Inhalt in Buffer */
			if (b->here+1 == b->end) {
				b->end = read(b->descriptor, b->buffer, linemax);
				if (b->end == 0) {
					return -1;
				} else if (b->end <0) {
					perror("Lesefehler\n");
					return -42;
				}	
				b->bytesread += b->end;
				b->here = 0;
				
				b->here--;
			}
			
		} else {
			hit_index = 0;
		}
		
		/* Gehe weiter */
		b->here++;
		
		/* Wenn Lineseparator komplett ist: */
		if (hit_index == b->lineseplen) {
			return b->here+b->bytesread-LINEBUFFERSIZE;
		}
	}
	

	
	
	/* Line-Ende */
	return -2;
}


/* Gibt Position des Bytestroms zurück */
int buf_where(LineBuffer *b) {
	return b->bytesread -b->end + b->here;
}

/* Positioniert auf Offset in Bytes */
int buf_seek(LineBuffer *b, int seekpos) {
	printf("BUF_SEEK (%d)\n",seekpos);
	b->bytesread = seekpos;
	b->here = 0;
	b->end = 0;
	return lseek(b->descriptor, seekpos*sizeof(char), SEEK_SET);
}

void print_pos(LineBuffer *b) {
	printf("Aktuelle Byteposition: %d\n", buf_where(b));
}

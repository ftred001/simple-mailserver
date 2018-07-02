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
 */
 
LineBuffer *buf_new(int descriptor, const char *linesep) {
	LineBuffer *lb;
	
	printf(">>>buf_new(%d)\n", descriptor);
	if ((lb = (LineBuffer*)calloc(1,sizeof(LineBuffer))) == NULL) {
		perror("Beim Allokieren des LineBuffers");
		return NULL;
	}
	
	lb->descriptor = descriptor;
	lb->linesep = linesep;
	lb->lineseplen = strlen(linesep);
	return lb;
}

void buf_dispose(LineBuffer *lb) {
	printf(">>>buf_dispose(fd: %d)\n", lb->descriptor); 
	
	if (lb == NULL) {
		perror("Nothing to dispose");
	}
	
	free(lb);

	/* printf("---Disposing succesful!\n"); */
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

int fill_buffer(LineBuffer *b, int linemax) {
	/* Wenn ich am Anfang oder Ende der LINE bin, lese ich eine neue LINE ein. */
	
	
	if (b->here == b->end) {
		/* printf("FILL BUFFER\n"); */
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
		/* print_buffer(b); */
	} 
	
	return b->end;
}

int buf_readline(LineBuffer *b, char *line, int linemax) {
	int hit_index=0;
	int li=0;
	int i;
	int res;
		
	if ((res = fill_buffer(b, linemax)) <0) {
		return res;
	}
	
	li = strlen(line);
	
	/* Suche Zeilenumbruch und gib Zeilenanfang zurück */
	while (b->here < b->end) {
		/* Kopiere Zeichen von Buffer in Line */
		line[li] =  b->buffer[b->here];
		
		/* Finde Vorkommen des LineSeparators */
		if (b->buffer[b->here] == b->linesep[hit_index]) {
			hit_index++;
		} else {
			hit_index = 0;
		}
		
		/* Gehe weiter */
		b->here++;
		li++;
		
		/* Wenn Lineseparator komplett ist: */
		if (hit_index == b->lineseplen) {
			for (i=0;i<=b->lineseplen; i++) {
				line[li-i] = '\0';
				/* printf("CUT Separator: %s\n", line); */
				
			}
			li = 0;
			return buf_where(b);
		}
		
		/* Wenn Zeile nicht komplett fülle den Buffer ein */
		if (b->here == b->end) {
			fill_buffer(b, linemax);
		}
	}
	printf("END OF FILE\n");
	return -1;
}


/* Gibt Position des Bytestroms zurück */
int buf_where(LineBuffer *b) {
	return b->bytesread-b->end + b->here;
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

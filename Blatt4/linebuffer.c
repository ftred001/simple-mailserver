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


int buf_readline(LineBuffer *b, char *line, int linemax) {
	int i;
	/* 
	 * return -1 bei EOF
	 * return OFFSETPOSITION des Bytestroms (in der Datei)
	 * ohne lseek?
	 * todo: Zeilentrenner CL RF über verschiedenen lines zulassen/merken
	 * */
	
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
		printf("RESET Buffer || here: %d end: %d\n", b->here, b->end);
	} else if (b->here > b->end) {
		printf("HERE IST GROESSER ALS END!\n");
	}
	
	
	/* Suche Zeilenumbruch und gib Position zurück */
	while (b->here < b->end) {
		/* TO DO: Ersten Teil des LineSeparators testen. Wenn enthalten und am ZeilenEnde dann merken.*/
		/* NOCH: Vergleicht mit komplettem Separator. */
		if (!strncmp(b->buffer+b->here,b->linesep, b->lineseplen)) {
			printf("Match gefunden bei: %d\n", b->bytesread+b->here);
			printf("Restbuffer: %s HERE: %d\n", b->buffer+b->here, b->here);
			return b->here++;
		}
		b->here++;
		
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

/*
int main(int argc, char *argv[]) {
	const char *line_separator = ".\r\n";
	char *line = calloc(0, LINEBUFFERSIZE);
	LineBuffer *lbuffer;
	int fd;
	
	printf("Lese folgende Datei ein: %s\n\n", argv[1]);
	
	fd = open(argv[1], O_RDONLY, 0644);
	
	lbuffer = buf_new(fd, line_separator);
		
	buf_seek(lbuffer, 25);
	while (buf_readline(lbuffer, line, LINEBUFFERSIZE)) {
		 print_buffer(lbuffer);
		 print_pos(lbuffer);
	} 
	
	buf_dispose(lbuffer);
	
	close(fd);
	
	return 0;
} */

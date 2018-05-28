#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> /* open() */
#include <unistd.h> /* read, write, close */
#include <sys/types.h> /* lseek */
#include <errno.h>
#include "linebuffer.h"

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
	
	/* Wenn ich am Anfang oder Ende der Zeile bin, lese ich eine neue Zeile ein. */
	if (b->here == b->end) {
		b->end = read(b->descriptor, b->buffer, linemax);
		if (b->end == 0) {
			/* Dateiende */
			printf("---END OF FILE---\n\n");
			return 0;
		} else if (b->end <0) {
			perror("Lesefehler\n");
			return -42;
		}	
		b->bytesread += b->end;
		b->here = 0;
	}
	
	/* Suche Zeilenumbruch und gib Position zurück */
	while (b->here < b->end) {
		b->here++;
		if (!strncmp(b->buffer+b->here,b->linesep, b->lineseplen)) {
			return b->here;
		}
	}
	
	/* Eingabeende */
	return -1;
}



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

int main(int argc, char *argv[]) {
	const char *line_separator = "\n";
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
}

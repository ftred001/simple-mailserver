#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> /* open() */
#include <unistd.h> /* read, write, close */
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
	printf("Descriptor: %d\n",lb->descriptor);
	if (!strcmp("\r\n", lb->linesep)) {
		printf("Separator: \\r\\n Length: %d\n", lb->lineseplen);
	}
	
	if (!strcmp("\n", lb->linesep)) {
		printf("Separator: \\n Length: %d\n", lb->lineseplen);
	}

	if (!strcmp("\r", lb->linesep)) {
		printf("Separator: \\r Length: %d\n", lb->lineseplen);
	}
	
	printf("Buffer: %s\n", lb->buffer);

	printf("bytesread %u | here: %u | end: %u\n", lb->bytesread, lb->here, lb->end);
	printf("---LINEBUFFER END---\n\n");
}


int buf_readline(LineBuffer *b, char *line, int linemax) {
	
	/* Wenn ich am Anfang oder Ende der Zeile bin, lese ich eine neue Zeile ein. */
	if (b->here == b->end) {
		b->end = read(b->descriptor, b->buffer, linemax);
		if (b->end == 0) {
			/* Dateiende */
			return 0;
		} else if (b->end <0) {
			perror("Lesefehler\n");
			return -1;
		}	
		b->bytesread += b->end;
	}
	
	/* Suche Zeilenumbruch und gib Position zurück */
	while (b->here < b->end) {
		b->here++;
		if (!strncmp(b->buffer+b->here,b->linesep, b->lineseplen)) {
			printf("ZEILENUMBRUCH GEFUNDEN an Stelle: %u!\n",b->here);
			return b->here;
		}
	}
	
	/* Eingabeende */
	return b->here;
}



int buf_where(LineBuffer *b) {
	return 0;
}

int main(int argc, char *argv[]) {
	const char *line_separator = "\r\n";
	char *line = calloc(0, LINEBUFFERSIZE);
	LineBuffer *lbuffer;
	int fd;
	
	printf("Lese folgende Datei ein: %s\n\n", argv[1]);
	
	fd = open(argv[1], O_RDONLY, 0644);
	
	lbuffer = buf_new(fd, line_separator);
	

	
	buf_readline(lbuffer, line, LINEBUFFERSIZE);
	print_buffer(lbuffer);
	
	buf_readline(lbuffer, line, LINEBUFFERSIZE);
	print_buffer(lbuffer);
	
	buf_readline(lbuffer, line, LINEBUFFERSIZE);
	print_buffer(lbuffer);

	
	buf_dispose(lbuffer);
	
	close(fd);
	
	return 0;
}

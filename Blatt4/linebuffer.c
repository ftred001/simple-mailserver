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
}

int buf_readline(LineBuffer *b, char *line, int linemax) {
	int gelesen;
	char *wort;
	
	printf("Lese Datei für Filedescriptor: %d\n", b->descriptor);
	
	while(1) {
		gelesen = read(b->descriptor, b->buffer, linemax);
		if (gelesen == 0) {
			break;
		} else if (gelesen <0) {
			perror("Lesefehler\n");
			break;
		}
		printf("Gelesen: %s\n", b->buffer);
		wort = b->buffer;
		b->end = strlen(b->buffer);
		b->bytesread += b->end;
		printf("Bytes read: %d HERE: %d END: %d\n", b->bytesread, b->here, b->end);
		
		/* finde Zeilenumbruch 
		while (b->here < b->end) {
			if (strncmp(b->buffer,b->linesep)) {
				printf("Zeilenumbruch an Pos: %d\n", b->here);
			}
			b->here++;
		} */
		
		while (*wort) {
			if (!strncmp(wort,b->linesep, b->lineseplen)) {
				printf("Zeilenumbruch an Stelle: %d\n", b->here);
			}
			b->here++;
			wort++;
		}
	
	}
	
	return 1;
}

void print_buffer(LineBuffer *lb) {
	printf("Descriptor: %d Separator-Length: %d\n",lb->descriptor, lb->lineseplen);
}

int main(int argc, char *argv[]) {
	const char *line_separator = "\n";
	char *line = calloc(0, LINEBUFFERSIZE);
	LineBuffer *lbuffer;
	int fd;
	
	printf("Lese folgende Datei ein: %s\n", argv[1]);
	
	fd = open(argv[1], O_RDONLY, 0644);
	
	lbuffer = buf_new(fd, line_separator);
	
	print_buffer(lbuffer);
	
	buf_readline(lbuffer, line, LINEBUFFERSIZE);
	
	buf_dispose(lbuffer);
	
	close(fd);
	
	return 0;
}

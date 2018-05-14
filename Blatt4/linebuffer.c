#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linebuffer.h"

LineBuffer *buf_new(int descriptor, const char *linesep) {
	LineBuffer *lb;
	lb = calloc(0,sizeof(LineBuffer));
	lb->descriptor = descriptor;
	lb->linesep = linesep;
	lb->lineseplen = strlen(linesep);
	return lb;
}

void print_buffer(LineBuffer *lb) {
	printf("Descriptor: %d Separator-Length: %d\n",lb->descriptor, lb->lineseplen);
}

int main(void) {
	const char *line_separator = "\n";
	LineBuffer *lbuffer;
	int fd = 0;
	
	lbuffer = buf_new(fd, line_separator);
	
	print_buffer(lbuffer);
	
	return 0;
}

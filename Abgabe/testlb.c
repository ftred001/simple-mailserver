#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "linebuffer.h"


int main(void) {
	const char *separator = "\n";
	const char *separator2 = "A";
	int fd = STDIN_FILENO;
	LineBuffer *b;
	
	printf("Create Buffer\n");
	b = buf_new(fd, separator);
	
	print_buffer(b);
	
	printf("Dispose Buffer\n");
	buf_dispose(b);
	
	
	return 0;
}

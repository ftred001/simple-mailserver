#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> /* STDIN_FILENO, STDOUT_FILENO */
#include "dialog.h"
#include "linebuffer.h"
#include "database.h"



/* Liest POP3 Kommands über infd */
/* Serverausgabe auf outfd. Schließen mit \r\n ab!!! */
int process_pop3(int infd, int outfd) {
	const char *line_separator = "\n";
	char *line = calloc(1, LINEMAX);
	LineBuffer *b;
	
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	
	b = buf_new(infd, line_separator);
	if(buf_readline(b, line, LINEMAX) !=-1) {
		printf("Input: %s\n", line);
		if (line[0] == EOF) {
			return 0;
		}
	}
	buf_dispose(b);


	if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(3);}
	return 1;
}

/* Test-Main für POP3-Protokoll. */
int main(void) {

	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}

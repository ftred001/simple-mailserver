#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> /* STDIN_FILENO, STDOUT_FILENO */
#include "dbm.h"
#include "linebuffer.h"


/* Liest POP3 Kommands über infd */
/* Serverausgabe auf outfd. Schließen mit \r\n ab!!! */
int process_pop3(int infd, int outfd) {
	int gelesen;
	const char IN_SEPARATOR = "\n";
	const char END_SEPARATOR = "\r\ņ";
	char line* = calloc(1, LINE_MAX);
	LineBuffer *b;
	
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	LineBuffer *b = buf_new(infd, IN_SEPARATOR);
	buf_readline(b, line, LINE_MAX);
	
	


	if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(3);}
	
	while(1) {
		gelesen = read(infd, buffer, BUFFSIZE);
	}
	
	
}

/* Test-Main für POP3-Protokoll. */
int main(void) {

	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}

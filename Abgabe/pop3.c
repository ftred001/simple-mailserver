#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> /* STDIN_FILENO, STDOUT_FILENO */
#include <string.h>

#include "linebuffer.h"
#include "dialog.h"
#include "database.h"

int state = 0;
const char *STD_FILEPATH = "serversettings.cfg";


int match_filter(DBRecord *rec, const void *data) {
	if (strlen(data) == 0) {
		return 2;
	}
	
	if (strstr(rec->key, data)) {
		return 1;
	}
	if (strstr(rec->cat, data)) {
		return 1;
	}
	return 0;
}


/* Liest POP3 Kommands über infd */
/* Serverausgabe auf outfd. Schließen mit \r\n ab!!! */
int process_pop3(int infd, int outfd) {
	ProlResult res;
	DBRecord *rec = calloc(1, sizeof(DBRecord));
	char *username = calloc(1, sizeof(char) * LINEMAX);
	const char *line_separator = "\n";
	char *line = calloc(1, LINEMAX);
	LineBuffer *b;
	
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	
	b = buf_new(infd, line_separator);
	if(buf_readline(b, line, LINEMAX) !=-1) {
		printf("Input: %s\n", line);
		
		/* Abbruchbedingung */
		if (line[0] == EOF) {
			return 0;
		}
		
		/* Schneide \n ab */
		line[strlen(line)] = '\0';
		
		res = processLine(line, state, dialog);
	
		printRes(res);
		printf("Param: %s\n", res.dialogrec->param);
		
		if (!strcmp(res.dialogrec->command, "list")) {
			printf("===LIST ALL===\n");
			db_list(STD_FILEPATH, outfd, match_filter, "");
			
		}
		
		if (!strcmp(res.dialogrec->command, "user")) {
			strcpy(rec->key, res.dialogrec->param);
			strcpy(rec->cat, "mailbox");
			db_search(STD_FILEPATH, 0, rec);
			
			printf("Result mbox: %s\n", rec->value);
			strcpy(username, rec->value);
		}
		
		if (!strcmp(res.dialogrec->command, "pass")) {
			strcpy(rec->key, username);
			strcpy(rec->cat, "password");
			db_search(STD_FILEPATH, 0, rec);
			
			printf("Result Password: %s\n", rec->value);
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

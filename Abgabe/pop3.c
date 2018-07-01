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
char *username;
char *mailbox;

DialogRec dialogspec[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{ "list", 		"",		0,		0,			validate_noparam },
	{ "user", 		"",		0,		1,			validate_hasparam },
	{ "list", 		"",		1,		1,			validate_noparam },
	{ "pass",		"",		1,		2,			validate_hasparam },
	{ "stat", 		"", 	2,		2,			validate_noparam },
	{ "list", 		"", 	2,		2,			},
	{ "retr", 		"",		2,		2,			validate_hasparam},
	{ "QUIT",		"",		2,		3,			validate_noparam },
	{ "" }
};


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
	ProlResult prolRes;
	DBRecord *dbRec = calloc(1, sizeof(DBRecord));
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
		
		prolRes = processLine(line, state, dialogspec);
        
        printf("PROL RESULT NACH PROCESSLINE\n");
        printRes(prolRes);
		
		
		if (!strcmp(prolRes.dialogrec->command, "list")) {
			printf("===LIST ALL===\n");
			db_list(STD_FILEPATH, outfd, match_filter, "");
			
		}
		
		if (!strcmp(prolRes.dialogrec->command, "user")) {
			strcpy(dbRec->key, prolRes.dialogrec->param);
			strcpy(dbRec->cat, "mailbox");
			db_search(STD_FILEPATH, 0, dbRec);
			
			printf("prolprolResult mbox: %s\n", dbRec->value);
            username = calloc(1, sizeof(char) * LINEMAX);
			strcpy(username, dbRec->value);
		}
		
		if (!strcmp(prolRes.dialogrec->command, "pass")) {
			strcpy(dbRec->key, username);
			strcpy(dbRec->cat, "password");
			db_search(STD_FILEPATH, 0, dbRec);
			
			printf("Result Password: %s\n", dbRec->value);
		}
		
		
	}

	buf_dispose(b);
    free(dbRec);


	if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(3);}
	return 1;
}

/* Test-Main für POP3-Protokoll. */
int main(void) {
    printf("POP3 Server started.\nAvailable Commands:list\nuser\npass\n");
	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}

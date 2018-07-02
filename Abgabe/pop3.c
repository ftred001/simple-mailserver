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
#include "fileindex.h"

int state = 0;
const char *STD_FILEPATH = "mailserver.db";
char *username;
char *mailbox;
int mailcount = 0;
FileIndex *file_index;

DialogRec dialogspec[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{ "user", 		"",		0,		1,			validate_hasparam },
	{ "pass",		"",		1,		2,			validate_hasparam },
	{ "stat", 		"", 	2,		2,			validate_noparam },
	{ "list", 		"", 	2,		2,			},
	{ "retr", 		"",		2,		2,			validate_hasparam},
	{ "QUIT",		"",		2,		3,			validate_noparam },
	{ "" }
};

void respond_with(int outfd, int status, char *msg) {
    
    if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(3);}
    
    if (status==1) {
        printf("-ERR \n");
    }
    
    if (status == 0) {
        printf("+OK ");
    }
    
    
    
    
}

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
    int db_index;
	DBRecord *dbRec = calloc(1, sizeof(DBRecord));
	const char *line_separator = "\n";
	char *line = calloc(1, LINEMAX);
	LineBuffer *b;
    FileIndexEntry *fi_entry;
    int msgno;

    
	
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	
	b = buf_new(infd, line_separator);
	if(buf_readline(b, line, LINEMAX) !=-1) {
		printf("Input: %s\n", line);
		
		/* Abbruchbedingung */
		if (line[0] == EOF) {
			return 0;
		}

		line[strlen(line)] = '\0';
		
		prolRes = processLine(line, state, dialogspec);
        
        if (prolRes.dialogrec != NULL) {
            state = prolRes.dialogrec->nextstate;
            printRes(prolRes);
            
            /* 0 User Login */
            if (!strcmp(prolRes.dialogrec->command, "user")) {
                strcpy(dbRec->key, prolRes.dialogrec->param);
                strcpy(dbRec->cat, "mailbox");
                write(outfd, "+OK", 4);
                if ((db_index = db_search(STD_FILEPATH, 0, dbRec)>=0)) {
                    username = calloc(1, sizeof(char) * LINEMAX);
                    strcpy(username, prolRes.dialogrec->param);
                    mailbox = calloc(1, sizeof(char) * LINEMAX);
                    strcpy(mailbox, dbRec->value);
                    

                    
                } else {
                    printf("User nicht gefunden!\n");
                    state=0;
                }
                
            }
            
            /* 1 Passwort */
            if (!strcmp(prolRes.dialogrec->command, "pass")) {
                strcpy(dbRec->key, username);
                strcpy(dbRec->cat, "password");
                db_search(STD_FILEPATH, 0, dbRec);
                
                printf("prolResult pass: %s\n", dbRec->value);
                if (!strcmp(prolRes.dialogrec->param, dbRec->value)) {
                    printf("+OK Logged in\n");
                    if ((file_index = fi_new(mailbox, line_separator)) == NULL) {
						perror("Init FileIndex");
					}
                    
                } else  {
                    printf("-ERR Passwörter stimmen nicht überein!\n");
                    state -=2;
                }
            }
            
            /* 2 stat */
            if (!strcmp(prolRes.dialogrec->command, "stat")) {
                printf("===STAT===\nvon Mailbox: %s\n", mailbox);               
                printf("+OK %d %d\n", file_index->nEntries, file_index->totalSize);
                
            }
            
            /* 2 list */
            if (!strcmp(prolRes.dialogrec->command, "list") && (!strlen(prolRes.dialogrec->param))) {
                printf("===LIST ALL===\n");
                
                if (file_index == NULL) {
                    file_index = fi_new(mailbox, line_separator);
                } else {
                    fi_dispose(file_index);
                    file_index = fi_new(mailbox, line_separator);
                }
                
                if (file_index->entries == NULL) {
                    perror("Keine Mails vorhanden!");
                } else {
					printf("+OK %d messages:\n", file_index->nEntries);
                    fi_entry = file_index->entries;
                    
                    while(fi_entry) {
                        printf("%d %d\r\n", fi_entry->nr, fi_entry->size);
                        fi_entry = fi_entry->next;
                    }
                    printf(".\r\n");
                }
                
            }
            
            /* 2 list msgno */
            if (!strcmp(prolRes.dialogrec->command, "list") && (strlen(prolRes.dialogrec->param))) {
                msgno = atoi(prolRes.dialogrec->param);
                printf("===LIST MSG %d===\n", msgno);
                
                if (file_index == NULL) {
                    file_index = fi_new(mailbox, line_separator);
                }
                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry != NULL) {
                    printf("+OK %d %d\n", fi_entry->nr, fi_entry->size);
                    /* Mit LSEEK oder BUF OFFSET positionieren und dann ausgeben. */
                    
                    printf(".\r\n");
                }
                
            }
            
            /* 2 retr msgno */
            if (!strcmp(prolRes.dialogrec->command, "retr")) {
                msgno = atoi(prolRes.dialogrec->param);
                printf("===RETR MSG #%d ===\n", msgno);
                
                if (file_index == NULL) {
                    file_index = fi_new(mailbox, line_separator);
                }
                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry != NULL) {
                    printf("+OK %d octets\n", fi_entry->size);
                    /* Mit LSEEK oder BUF OFFSET positionieren und dann ausgeben. */
                    
                    printf(".\r\n");
                }
                
                
            }
            
            /* 2 quit */
            if (!strcmp(prolRes.dialogrec->command, "quit")) {
                printf("===QUIT===\n");
                fi_dispose(file_index);
            }
            
            
        } else {
            printf("ProlRes.dialogrec == NULL!\n");
        }
	}

	buf_dispose(b);
    free(dbRec);
    
    printf("====\nState: %d\n====\n", state);

	
	return 1;
}

/* Test-Main für POP3-Protokoll. */
int main(void) {
    printf("POP3 Server started.\nAvailable Commands:list\nuser\npass\nstat\nlist\nretr\n");
	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}

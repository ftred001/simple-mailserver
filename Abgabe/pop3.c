#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> /* STDIN_FILENO, STDOUT_FILENO, write, read */
#include <string.h>
#include <strings.h> /* strcasecmp() */

#include "linebuffer.h"
#include "dialog.h"
#include "database.h"
#include "fileindex.h"
#include "pop3.h"

int state = 0;
const char *STD_FILEPATH = "mailserver.db";
char *username;
char *mailbox;
int mailcount = 0;
FileIndex *file_index;
const char *msg_separator = "\r\n";
int running = 0;

DialogRec dialogspec[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{ "user", 		"",		0,		1,			validate_hasparam },
	{ "pass",		"",		1,		2,			validate_hasparam },
	{ "stat", 		"", 	2,		2,			validate_noparam },
	{ "list", 		"", 	2,		2,			},
	{ "retr", 		"",		2,		2,			validate_hasparam },
	{ "noop",		"",		2,		2,			validate_noparam },
	{ "rset",		"",		2,		2,			validate_noparam  },
	{ "dele",		"",		2,		2,			validate_hasparam },
	{ "quit",		"",		2,		0,			validate_noparam },
	{ "" }
};


/* Liest POP3 Kommands über infd */
/* Serverausgabe auf outfd. Schließen mit \r\n ab!!! */
int process_pop3(int infd, int outfd) {
	ProlResult prolRes;
    int db_index;
	DBRecord *dbRec = calloc(1, sizeof(DBRecord));
	const char *line_separator = "\n";
	char *line = (char*)calloc(LINEMAX, sizeof(char));
	char *msg_line;
	char *response = (char*)calloc(LINEMAX, sizeof(char));
	LineBuffer *linebuf;
	LineBuffer *msg_buf;
    FileIndexEntry *fi_entry;
    int msgno;
    int msg_fd;
    int msg_i;

    
	
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(4);}
	
	if (running == 0) {
		sprintf(response,"+OK Halli Hallo auf dem fast guten Mailserver!\r\n");
		write(outfd,response,strlen(response)+1);
		running = 1;
	}

	
	linebuf = buf_new(infd, line_separator);
	if(buf_readline(linebuf, line, LINEMAX) !=-1) {
		/* printf("Input: %s\n", line); */
		
		/* Abbruchbedingung */
		if (line[0] == EOF) {
			return 0;
		}

		line[strlen(line)] = '\0';
		
		prolRes = processLine(line, state, dialogspec);
        
        if (prolRes.dialogrec != NULL) {
            state = prolRes.dialogrec->nextstate;
            /* printRes(prolRes); */
            
            /* 0 User Login */
            if (!strcasecmp(prolRes.dialogrec->command, "user")) {
                strcpy(dbRec->key, prolRes.dialogrec->param);
                strcpy(dbRec->cat, "mailbox");
                strcpy(response, "+OK\r\n");
                
                username = calloc(DB_KEYLEN, sizeof(char));
                mailbox = calloc(DB_VALLEN, sizeof(char));
                
                if ((db_index = db_search(STD_FILEPATH, 0, dbRec)>=0)) {

                    strcpy(username, prolRes.dialogrec->param);

                    strcpy(mailbox, dbRec->value);
                } else {
					memset(username, 0, DB_KEYLEN);
					memset(mailbox, 0, DB_VALLEN);
				}
            }
            
            /* 1 Passwort */
            if (!strcasecmp(prolRes.dialogrec->command, "pass")) {
				/*printf("Username: %s Password:%s\n", username, prolRes.dialogrec->param);*/
				
				if (strlen(username)) {
				    strcpy(dbRec->key, username);
				    strcpy(dbRec->cat, "password");
					
					if ((db_search(STD_FILEPATH, 0, dbRec) < 0)) {
						perror("Kein Ergebnis");
						sprintf(response, "-ERR\r\n");
					}
					
					
					if (!strcmp(prolRes.dialogrec->param, dbRec->value)) {
						if ((file_index = fi_new(mailbox, line_separator)) == NULL) {
							perror("Init FileIndex");
							sprintf(response, "-ERR\r\n");
						} else {
							sprintf(response, "+OK Logged in\r\n");
						}
                    
					} else  {
						/*printf("-ERR Passwörter stimmen nicht überein!\n");*/
						sprintf(response, "-ERR\r\n");
						state = 0;
					}
				} else  {
					sprintf(response, "-ERR\r\n");
					/*printf("-ERR User existiert nicht!\n");*/
					state = 0;
				}
            }
            

            
            /* 2 stat */
            if (!strcasecmp(prolRes.dialogrec->command, "stat")) {    
                sprintf(response, "+OK %d %d\r\n", file_index->nEntries, file_index->totalSize);
            }
            
            /* 2 list */
            if (!strcasecmp(prolRes.dialogrec->command, "list") && (!strlen(prolRes.dialogrec->param))) {
                                
                if (file_index->entries == NULL) {
                    perror("Keine Mails vorhanden!");
                    sprintf(response, "ERR\r\n");
                } else {
					sprintf(response, "+OK %d messages:\r\n", file_index->nEntries);
                    fi_entry = file_index->entries;
                    
                    while(fi_entry) {
						if (fi_entry->del_flag == 0) {
						    sprintf(response, "%d %d\r\n", fi_entry->nr, fi_entry->size);
							if (write(outfd, response, strlen(response)+1) <0) {
								perror("responding (write)");
							}
						}
                        fi_entry = fi_entry->next;
                    }
                    sprintf(response, ".\r\n");
                }
                
            }
            
            /* 2 list msgno */
            if (!strcasecmp(prolRes.dialogrec->command, "list") && (strlen(prolRes.dialogrec->param))) {
                msgno = atoi(prolRes.dialogrec->param);
                                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry != NULL) {
					if (fi_entry->del_flag == 0) {
						sprintf(response,"+OK %d %d\r\n", fi_entry->nr, fi_entry->size);  
					} else {
						sprintf(response,"-ERR\r\n");
					}
                                      
                }
                
            }
            
            /* 2 retr msgno */
            if (!strcasecmp(prolRes.dialogrec->command, "retr")) {
                msgno = atoi(prolRes.dialogrec->param);
                
                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry == NULL || fi_entry->del_flag == 1) {
					sprintf(response, "-ERR\r\n");
				} else {
                    sprintf(response, "+OK %d octets\r\n", fi_entry->size);
                    if (write(outfd, response, strlen(response)+1) <0) {
						perror("responding (write)");
					}
                    
                    
                   
                    /* neuer Buffer zum Ausgeben der Mail */
                    if ((msg_fd=open(mailbox, O_RDONLY, 0644))<0) {
						perror("Beim Öffnen des Filepaths");
					}
                    
                    msg_buf = buf_new(msg_fd,line_separator);
                    if (msg_buf == NULL) {
						perror("Beim Allokieren das MSG Buffers");
					}
                    
                    /* Buffer offsetten */
					if (buf_seek(msg_buf, fi_entry->seekpos) <0) {
						perror("BUF_SEEK <0");
					}
					
					/* Zeilenweise ausgeben */
					msg_i =1;
					msg_line = (char*)calloc(LINEMAX, sizeof(char));
					
					printf("PRINT RETR\n");
					
					while (((buf_readline(msg_buf, msg_line, LINEBUFFERSIZE)) !=-1) && (msg_i <= fi_entry->lines)) {
						sprintf(response, "%s\r\n",msg_line);
						
						if (write(outfd, response, strlen(response)+1) <0) {
							perror("Error bei Write");
						}
						
						msg_line[0] = '\0';
						msg_i++;
					}
                    sprintf(response,".\r\n");
                    close(msg_fd);
                    buf_dispose(msg_buf);
                }
                
            }
            
            /* 2 quit */
            if (!strcasecmp(prolRes.dialogrec->command, "quit")) {
                memset(username, 0, DB_KEYLEN);
                free(username);
                memset(mailbox, 0, DB_VALLEN);
                free(mailbox);
                fi_compactify(file_index);           
                fi_dispose(file_index);
                state = 0;
                sprintf(response, "+OK Logging out.\r\n");
            }
            
            
            /* 2 noop */
            if (!strcasecmp(prolRes.dialogrec->command, "noop")) {    
                sprintf(response, "+OK\r\n");
            }
            
            /* 2 dele msgno */
            if (!strcasecmp(prolRes.dialogrec->command, "dele") && (strlen(prolRes.dialogrec->param))) {
                msgno = atoi(prolRes.dialogrec->param);
                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry != NULL) {
					fi_entry->del_flag = 1;
					file_index->totalSize -= fi_entry->size;
					file_index->nEntries--;
                    sprintf(response,"+OK\r\n");                    
                } else {
					sprintf(response, "-ERR\r\n");
				}
                
            }
            
            
            
            
        } else {
            sprintf(response, "-ERR\r\n");
        }
	}
	
	/* Schicke Antwort an OUT_FD */
	if (write(outfd, response, strlen(response)+1) <0) {
		perror("CRITICAL! Couldnt write to OUTFD");
	}
	
	buf_dispose(linebuf);
    free(dbRec);
    memset(response, 0, LINEMAX);
    free(response);
    
    /* printf("====\nState: %d\n====\n", state); */

	
	return 1;
}


/* Test-Main für POP3-Protokoll. */
int main(void) {
    printf("POP3 Server started.\nAvailable Commands: list | user | pass | stat | list (#)| retr #\n");
	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}

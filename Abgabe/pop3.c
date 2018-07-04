#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h> /* STDIN_FILENO, STDOUT_FILENO, write, read */
#include <string.h>
#include <strings.h> /* strcasecmp() */

#include <sys/types.h>
#include <signal.h>

#include "linebuffer.h"
#include "dialog.h"
#include "database.h"
#include "fileindex.h"
#include "pop3.h"


const char *STD_FILEPATH = "mailserver.db";
char pop_lockfile[LINEMAX] = {0};
static int clientsocket;

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

void pop_sighandler(int sig) {
	remove(pop_lockfile);
	close(clientsocket);
	exit(1);
}

int lock_pop_fp(char *path) {
	int fd, bytes_read;
	char pid[20];
	
	fd = open(path, O_RDWR | O_CREAT, 0644);
	
	if(fd<1) {
		perror("Lockfile");exit(1);
	}
	
	bytes_read = read(fd, pid, 20);
	
	if (bytes_read > 0 && kill(atoi(pid), 0) ==0) {
		close(fd);
		return 0;
	}
	
	if (fd >= 0) {
		memset(&pid[0], 0, 20);
		sprintf(pid, "%d", getpid());
		write(fd, pid, strlen(pid));
	}
	
	close(fd);
	return 1;	
}

int unlock_pop_fp(char *path) {
	return (unlink(path) == 0);
}


/* Liest POP3 Kommands über infd */
/* Serverausgabe auf outfd. Schließen mit \r\n ab!!! */
int process_pop3(int infd, int outfd) {
	ProlResult prolRes;
    int db_index;
	DBRecord dbRec =  {"","",""};
	const char *line_separator = "\n";
	const char *MSG_SEPARATOR = "\r\n";
	char line[LINEMAX] ={0};
	char msg_line[LINEMAX] = {0};
	char response[LINEMAX] = {0};
	LineBuffer *msg_buf;
	LineBuffer *line_buffer;
    FileIndexEntry *fi_entry;
    int msgno;
    int msg_fd;
    int msg_i;
    char username[DB_VALLEN];
    char mailbox[DB_VALLEN];
    
    int state = 0;

	FileIndex *file_index = NULL;
	int running = 0;
	
    
    
    
	/* Starte Server */
	if (infd<0) { perror("Bei Oeffnen der Eingabedatei");exit(2);}
	if (outfd<0) { perror("Bei Oeffnen der Ausgabedatei");exit(4);}
	
	signal(SIGINT, pop_sighandler);
	
	if (running == 0) {
		sprintf(response,"+OK Halli Hallo auf dem fast guten Mailserver! ready.\r\n");
		write(outfd,response,strlen(response)+1);
		running = 1;
	}
	
	line_buffer = buf_new(infd, MSG_SEPARATOR);

	while(1) {
		memset(line, 0, LINEMAX);
		buf_readline(line_buffer, line, LINEMAX);
		line[strlen(line)] = '\0';
		
		printf("Line: %s\n",line);
		prolRes = processLine(line, state, dialogspec);
		
		
        
        if (!prolRes.failed) {
            
            printRes(prolRes);
            
            /* Standardverfahren */
			state = prolRes.dialogrec->nextstate;
			sprintf(response, "-ERR Command not found\r\n");
            

            /* 0 User Login */
            if (!strcasecmp(prolRes.dialogrec->command, "user")) {
                strcpy(dbRec.key, prolRes.dialogrec->param);
                strcpy(dbRec.cat, "mailbox");
                strcpy(response, "+OK\r\n");
                
                
                if ((db_index = db_search(STD_FILEPATH, 0, &dbRec)>=0)) {

                    strcpy(username, prolRes.dialogrec->param);
                    strcpy(mailbox, dbRec.value);
                    printf("User %s found\n", prolRes.dialogrec->param);
                    sprintf(pop_lockfile, "%s.lock", mailbox);
                     
                } else {
					printf("User %s not found\n", prolRes.dialogrec->param); 
				}
            }
            
            /* 1 Passwort */
            if (!strcasecmp(prolRes.dialogrec->command, "pass")) {
				printf("Username: %s Password:%s\n", username, prolRes.dialogrec->param);
				
				if (strlen(username)) {
				    strcpy(dbRec.key, username);
				    strcpy(dbRec.cat, "password");
					
					if ((db_search(STD_FILEPATH, 0, &dbRec) < 0)) {
						perror("Kein Ergebnis");
						sprintf(response, "-ERR no entry found on db\r\n");
					}
					
					
					if (!strcmp(prolRes.dialogrec->param, dbRec.value)) {
						if ((file_index = fi_new(mailbox, line_separator)) == NULL) {
							perror("Init FileIndex");
							sprintf(response, "-ERR coudlnt init fileindex\r\n");
						} else {
							if (!lock_pop_fp(pop_lockfile)) {
								sprintf(response, "-ERR User is locked in\r\n");
							} else {
								sprintf(response, "+OK Logged in\r\n");
							}
							
							
						}
                    
					} else  {
						sprintf(response, "-ERR passwords not matching\r\n");
						state = 0;
					}
				} else  {
					sprintf(response, "-ERR user doesnt exist\r\n");
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
                    sprintf(response, "-ERR no mails on server\r\n");
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
						sprintf(response,"-ERR couldnt find that msgno \r\n");
					}
                                      
                }
                
            }
            
            /* 2 retr msgno */
            if (!strcasecmp(prolRes.dialogrec->command, "retr")) {
                msgno = atoi(prolRes.dialogrec->param);
                
                
                fi_entry = fi_find(file_index, msgno);
                
                if (fi_entry == NULL || fi_entry->del_flag == 1) {
					sprintf(response, "-ERR no entries\r\n");
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
					
					/*
					printf("PRINT RETR\n");
					*/
					
					while (((buf_readline(msg_buf, msg_line, LINEBUFFERSIZE)) !=-1) && (msg_i <= fi_entry->lines)) {
						
						if (msg_i>1) {
							sprintf(response, "%s\r\n",msg_line);
						
							if (write(outfd, response, strlen(response)+1) <0) {
								perror("Error bei Write");
							}
						}

						
						msg_line[0] = '\0';
						msg_i++;
					}
                    sprintf(response,".\r\n");
                    close(msg_fd);
                    buf_dispose(msg_buf);
                }
                
            }
                       
            
            /* 2 noop */
            if (!strcasecmp(prolRes.dialogrec->command, "noop")) {    
                sprintf(response, "+OK\r\n");
            }
            
            /* 2 RSET */
            if (!strcasecmp(prolRes.dialogrec->command, "rset")) {
				/* Muss Compactify ausführen */
				    
                sprintf(response, "-ERR not implemented! \r\n");
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
					sprintf(response, "-ERR entry to delete not found\r\n");
				}
                
            }
            
			/* 2 quit */
            if (!strcasecmp(prolRes.dialogrec->command, "quit")) {
                fi_compactify(file_index);           
                fi_dispose(file_index);
                state = 0;
                unlock_pop_fp(pop_lockfile);
                
                sprintf(response, "+OK Logging out.\r\n");
                
                /* Schicke Antwort an OUT_FD */
				if (write(outfd, response, strlen(response)+1) <0) {
					perror("CRITICAL! Couldnt write to OUTFD");
				}
				
				
				
                return 0;
            }
		
		} else {
			printf("prolRes failed\n");
            sprintf(response, "-ERR ProlResult failed.\r\n");
        }
        		
		
		/* Schicke Antwort an OUT_FD */
		if (write(outfd, response, strlen(response)) <0) {
			perror("CRITICAL! Couldnt write to OUTFD");
		}

		
		/* printf("====\nState: %d\n====\n", state); */

        
	}
	
	if(file_index != NULL){
		fi_dispose(file_index);
	}
	
	buf_dispose(line_buffer);
	
	return 0;
}


/* Test-Main für POP3-Protokoll. 
int main(void) {
    printf("POP3 Server started.\nAvailable Commands: list | user | pass | stat | list (#)| retr #\n");
	while(process_pop3(STDIN_FILENO, STDOUT_FILENO)) {
		
	}
	
	return 0;
}*/

#include "smtp.h"
#include "database.h"
#include "dialog.h"
#include "fileindex.h"
#include "linebuffer.h"

#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

static int clientsockfd;
char sender[100];
char FROM[100];
char smtp_lockfile[LINEMAX] = {0};
char mailbox[255];
char message[PARAMMAX] = "";

int unlock_smtp_fp(char *pfad);

int validate_rcpt(DialogRec *d) {
	int index;
	int get;
	char rcpt[100];
	DBRecord empf = {"", "smtp", ""};
	char *end;
	char *start = strstr(d->param, "<");
	
	strcpy(rcpt, start+1);
	end = strstr(rcpt, ">");
	strcpy(end, "\0");
	
	strcpy(empf.key, rcpt);
	
	index = db_search("mailserver.db", 0, &empf);
	if(index < 0){
		strcpy(message, "550 unknown rcpt\r\n");
		return 0;
	}
	
	get = db_get("mailserver.db", index, &empf);
	if(get < 0){
		strcpy(message, "500 Fehler!\r\n");
		return 0;
	}
	
	memset(mailbox, 0, 255);
	strcpy(mailbox, empf.value);
	
	return 1;
}

DialogRec smtpDialog[] = {
	/* command, param, state, nextstate, validatosr */
	{"helo", "", 0, 1},
	{"mail from", "", 1, 2},
	{"rcpt to", "", 2, 3, validate_rcpt},
	{"data", "", 3, 4},
	{".", "", 4, 5},
	{"quit", "", 5, 0},
	{""}
};



void smtp_sighandler(int sig){
	/*remove(lockfilepath);*/
	unlock_smtp_fp(smtp_lockfile);
	close(clientsockfd);
	exit(1);
}

void create_FROM(){
	time_t t;
	
	memset(FROM, 0, 100);
	time(&t);
	sprintf(FROM, "From %s %s", sender, ctime(&t));
}

int lock_smtp_fp(char *pfad){
	int fd;
	int fdContent;
	char pid[100];
	fd = open(pfad, O_RDWR | O_CREAT, 0644);
	
	if(fd < 1) {
		perror("ERROR! Oeffnen fehlgeschlagen!");
		exit(1);
	}
	
	fdContent = read(fd, pid, 100);
	
	if(fdContent > 0 && kill(atoi(pid), 0) == 0){
		close(fd);
		return 0;
	}
	
	if (fd >= 0){
		memset(&pid[0], 0, sizeof(pid));
		sprintf(pid, "%d", getpid());
		write(fd, pid, strlen(pid));
	}
	
	close(fd);
	
	return 1;
}

int unlock_smtp_fp(char *pfad){
	return (unlink(pfad) == 0);
}


int append_to_mailbox(char *pfad){
	
	int filefd = open(pfad, O_WRONLY | O_APPEND, 0640);
	if(filefd < 0){
		perror("Bei Oeffnen der Eingabedatei");
		exit(2);
	}
	
	write(filefd, FROM, strlen(FROM));
	
	return filefd;
}


int process_smtp(int infd, int outfd){
	char buffer[LINEMAX];
	ProlResult result;
	char error[LINEMAX] = "500 Fehler!\r\n";
	char filepath[LINEMAX] = {0};
	int filefd;
	int state = 0;
	DBRecord db_record = {"", "", ""};
	int db_index;
	
	LineBuffer *linebuf;
	int readline;
	
	char *absEnd;
	char *absStart;
	
	clientsockfd = infd;
	
	signal(SIGINT, smtp_sighandler);
	
	write(outfd, "220 meinmailserver.de\r\n", 23);
	
	while(1){
		memset(buffer, 0, LINEMAX);
		
		if(state == 4){
			
			if(filefd < 0){
				perror("Bei Oeffnen der Eingabedatei");
				exit(2);
			}
			
			linebuf = buf_new(infd, "\r\n");
			
			while((readline = buf_readline(linebuf, buffer, LINEBUFFERSIZE)) != -1){
				if(!strcmp(buffer, ".")){
					result = processLine(buffer, state, smtpDialog);
					if(!result.failed){
						state = result.dialogrec->nextstate;
						write(filefd, "\n", 1);
						close(filefd);
						write(outfd, "250 Ok\r\n", strlen("250 Ok\r\n"));
					} else {
						write(outfd, error, strlen(error));
					}
					break;
				}
				strcat(buffer, "\n");
				write(filefd, buffer, strlen(buffer));
			}
			
			continue;
		}
		
		read(infd, buffer, sizeof(buffer));
		
		if(!strncasecmp(buffer, "helo", 4)){
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				write(outfd, "250 Ok\r\n", strlen("250 Ok\r\n"));
			} else {
				write(outfd, error, strlen(error));
			}
		}
		else if(!strncasecmp(buffer, "mail from", 9)){
			
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				memset(sender, 0, 100);
				absStart = strstr(buffer, "<");
	
				strcpy(sender, absStart+1);
				absEnd = strstr(sender, ">");
				strcpy(absEnd, "\0");
				
				write(outfd, "250 Ok\r\n", strlen("250 Ok\r\n"));
			} else {
				write(outfd, error, strlen(error));
			}
			
		}
		else if(!strncasecmp(buffer, "rcpt to", 7)){
			
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				strcpy(db_record.key, mailbox);
				
				db_index = db_search("mailserver.db", 0, &db_record);
				if(db_index >= 0){
					db_get("mailserver.db", db_index, &db_record);
				}
				
				strcpy(filepath, db_record.value);
				sprintf(smtp_lockfile, "%s.lock", mailbox);
				
				if(!lock_smtp_fp(smtp_lockfile)){
					write(outfd, "500 Fehler beim oeffnen\r\n", strlen("500 Fehler beim oeffnen\r\n"));
					exit(1);
				}
				
				write(outfd, "250 Ok\r\n", strlen("250 Ok\r\n"));
			} else {
				write(outfd, result.info, strlen(result.info));
			}
			
		}
		else if(!strncasecmp(buffer, "data", 4)){
			
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				
				create_FROM();
				filefd = append_to_mailbox(filepath);
				
				write(outfd, "354 End data with <CR><LF>.<CR><LF>\r\n", strlen("354 End data with <CR><LF>.<CR><LF>\r\n"));
			} else {
				write(outfd, error, strlen(error));
			}
			
		}
		else if(!strncasecmp(buffer, ".", 1)){
			
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				write(outfd, "250 Ok\r\n", strlen("250 Ok\r\n"));
			} else {
				write(outfd, error, strlen(error));
			}
			
		}
		else if(!strncasecmp(buffer, "quit", 4)){
			
			result = processLine(buffer, state, smtpDialog);
			if(!result.failed){
				state = result.dialogrec->nextstate;
				unlock_smtp_fp(smtp_lockfile);
				write(outfd, "221 Bye\r\n", strlen("221 Bye\r\n"));
			} else {
				write(outfd, error, strlen(error));
			}
			break;
		} else {
			write(outfd, error, strlen(error));
		}
	}
	
	return 0;
}

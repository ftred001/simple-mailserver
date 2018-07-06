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

static int clientsocket;
char sender[100];
char FROMLINE[100];
char smtp_lockfile[MAXDATEIPFAD] = {0};
char mailbox[255];
char message[PARAMMAX] = "";



int lock_smtp_fp(char *pfad){
	int fd, bytes_read;
	char pid[20];
	
	if((fd = open(pfad, O_RDWR | O_CREAT, 0644)) < 1) {
		perror("Oeffne FD");
		exit(1);
	}
	
	bytes_read = read(fd, pid, 20);
	
	if(bytes_read > 0 && kill(atoi(pid), 0) == 0){
		close(fd);
		return 0;
	}
	
	if (fd >= 0){
		memset(&pid[0], 0, 20);
		sprintf(pid, "%d", getpid());
		write(fd, pid, strlen(pid));
	}
	close(fd);
	return 1;
}

int unlock_smtp_fp(const char *pfad){
	return (unlink(pfad) == 0);
}

void smtp_sighandler(int sig){
	unlock_smtp_fp(smtp_lockfile);
	close(clientsocket);
	exit(1);
}

int validate_rcpt(DialogRec *d) {
	int index;
	int get;
	char rcpt[80];
	DBRecord empfaenger_rec = {"", "smtp", ""};
	char *end;
	char *start = strstr(d->param, "<");
	
	strcpy(rcpt, start+1);
	end = strstr(rcpt, ">");
	strcpy(end, "\0");
	
	strcpy(empfaenger_rec.key, rcpt);
	
	index = db_search("mailserver.db", 0, &empfaenger_rec);
	if(index < 0){
		strcpy(message, "550 unknown rcpt\r\n");
		return 0;
	}
	
	get = db_get("mailserver.db", index, &empfaenger_rec);
	if(get < 0){
		strcpy(message, "500 Fehler!\r\n");
		return 0;
	}
	
	memset(mailbox, 0, 255);
	strcpy(mailbox, empfaenger_rec.value);
	
	return 1;
}


DialogRec SMTP_DIALOG[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{"helo", 		"", 	0, 		1},
	{"mail from", 	"", 	1, 		2},
	{"rcpt to", 	"", 	2, 		3, 		validate_rcpt},
	{"data", 		"", 	3, 		4},
	{".", 			"", 	4, 		5},
	{"quit", 		"", 	5, 		0},
	{""}
};





void create_fromline(){
	time_t t;
	memset(FROMLINE, 0, 100);
	time(&t);
	sprintf(FROMLINE, "FROM %s %s", sender, ctime(&t));
}



/* Gibt FD für das Anhängen an Mailbox zurück */
int append_to_mailbox(const char *pfad){
	int fd = open(pfad, O_WRONLY | O_APPEND, 0640);
	if(fd < 0){
		perror("Bei Oeffnen der Eingabedatei");
		exit(2);
	}
	write(fd, FROMLINE, strlen(FROMLINE));
	return fd;
}


int process_smtp(int infd, int outfd){
	char line[LINEMAX];
	ProlResult prolRes;
	char error[LINEMAX] = "500 Fehler!\r\n";
	char filepath[MAXDATEIPFAD] = {0};
	int datafile;
	int state = 0;
	DBRecord db_record = {"", "", ""};
	int db_index;
	
	LineBuffer *linebuf;
	int umbruch;
	
	char *absender_end;
	char *absender_start;
	
	char response[LINEBUFFERSIZE] = {0};
	
	clientsocket = infd;
	
	signal(SIGINT, smtp_sighandler);
	
	write(outfd, "220 meinsmtpserver.de\r\n", strlen("220 meinsmtpserver.de\r\n")+1);
	
	
	
	while(1){
		memset(line, 0, LINEMAX);
		
		if(state == 4){
			
			if(datafile < 0){
				perror("Bei Oeffnen der datafile");
				exit(2);
			}
			
			
			
			linebuf = buf_new(infd, "\r\n");
			
			while((umbruch = buf_readline(linebuf, line, LINEBUFFERSIZE)) != -1){
				if(!strncmp(line, ".",1)){
					prolRes = processLine(line, state, SMTP_DIALOG);
					if(!prolRes.failed){
						state = prolRes.dialogrec->nextstate;
						write(datafile, "\n", 1);
						close(datafile);
						sprintf(response, "250 Ok\r\n");
					} else {
						printf("LineBuff error");
						write(outfd, error, strlen(error));
					}
					break;
				}
				strcat(line, "\n");
				write(datafile, line, strlen(line));
			}
			
			continue;
		}
		
		read(infd, line, sizeof(line));
		
		/* HELO */
		if(!strncasecmp(line, "helo",4)){
			printf("HELO\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				sprintf(response, "250 Ok\r\n");
			} else {
				sprintf(response, "%s",error);

			}
		}
		
		/* MAIL FROM */
		else if(!strncasecmp(line, "mail from",9)){
			printf("MAIL FROM\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			printf("%s %s\n", prolRes.dialogrec->param, prolRes.dialogrec->command);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				memset(sender, 0, 100);
				absender_start = strstr(line, "<");
	
				strcpy(sender, absender_start+1);
				absender_end = strstr(sender, ">");
				strcpy(absender_end, "\0");
				sprintf(response, "250 Ok\r\n");
			} else {
				sprintf(response, "%s", error);
			}
		}
		
		/* RCPT TO */
		else if(!strncasecmp(line, "rcpt to",7)){
			printf("rcpt to\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				strcpy(db_record.key, mailbox);
				
				db_index = db_search("mailserver.db", 0, &db_record);
				if(db_index >= 0){
					db_get("mailserver.db", db_index, &db_record);
				}
				
				strcpy(filepath, db_record.value);
				sprintf(smtp_lockfile, "%s.lock", mailbox);
				
				if(!lock_smtp_fp(smtp_lockfile)){
					sprintf(response, "500 file locked.\r\n");
					exit(1);
				}
				sprintf(response, "250 Ok\r\n");
			} else {
				sprintf(response, "%s\r\n", prolRes.info);
			}
			
		}
		
		/* DATA */
		else if(!strncasecmp(line, "data",4)){
			printf("data\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				
				create_fromline();
				datafile = append_to_mailbox(filepath);
				sprintf(response, "354 End data with <CR><LF>.<CR><LF>\r\n");

			} else {
				sprintf(response, "%s", error);
			}
			
		}
		
		/* . */
		else if(!strncasecmp(line, ".",1)){
			printf(".\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				sprintf(response,"250 Ok\r\n");
			} else {
				sprintf(response, "%s", error);
			}
			
		}
		
		/* QUIT */
		else if(!strncasecmp(line, "quit", 4)){
			printf("quit\n");
			prolRes = processLine(line, state, SMTP_DIALOG);
			if(!prolRes.failed){
				state = prolRes.dialogrec->nextstate;
				unlock_smtp_fp(smtp_lockfile);
				sprintf(response, "221 Bye\r\n");
			} else {
				sprintf(response, "%s", error);
			}
			write(outfd, response, strlen(response));
			break;
		} else {
			sprintf(response,"%s", error);
		}
		
		write(outfd, response, strlen(response));
		
	}
	
	return 0;
}

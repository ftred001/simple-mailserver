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
char sender[100] = {0};
char recipient[80]= {0};
char FROMLINE[100];
char smtp_lockfile[MAXDATEIPFAD] = {0};
char mailbox[MAXDATEIPFAD];
char valid_msg[PARAMMAX] = "";

int lock_smtp_fp(char *pfad){
	int fd, bytes_read;
	char pid[20];
	
	
	if((fd = open(pfad, O_RDWR | O_CREAT, 0644)) < 1) {
		perror("Create or open Lockfile");
		exit(1);
	}
	
	bytes_read = read(fd, pid, 20);
	printf("LOCK PID:%s\n", pid);
	
	if(bytes_read > 0 && kill(atoi(pid), 0) == 0){
		close(fd);
		return 0;
	}
	
	/* Schreibe Process ID in Lockfile */
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

/* Validiert dass die Mailbox auch auf der Datenbank exisitert. */ 
int validate_rcpt(DialogRec *d) {
	int index;
	int get;
	char rcpt[80];
	DBRecord recipient_rec = {"", "smtp", ""};
	
	/* Kopiere Mailadresse in den Key */
	strcpy(rcpt, d->param);
	if (rcpt[strlen(rcpt)-1] == '>') {
		rcpt[strlen(rcpt)-1] = '\0';
	} else {
		perror("SYNTAX ERROR");
		return 0;
	}
	
	strcpy(recipient_rec.key, rcpt);
	
	index = db_search("mailserver.db", 0, &recipient_rec);
	if(index < 0){
		perror("Empfänger Key unbekannt");
		strcpy(valid_msg, "550 Empfänger unbekannt.\r\n");
		return 0;
	}
	
	get = db_get("mailserver.db", index, &recipient_rec);
	if(get < 0){
		perror("Empfänger Value fehlerhalft");
		strcpy(valid_msg, "500 Konnte DB Eintrag nicht auslesen.\r\n");
		return 0;
	}
	
	memset(mailbox, 0, MAXDATEIPFAD);
	strcpy(mailbox, recipient_rec.value);
	
	return 1;
}


DialogRec SMTP_DIALOG[] = {
	/* Command,		Param, 	State,	Next-State,	Validator */
	{"helo", 		"", 	0, 		1},
	{"mail from:<", "", 	1, 		2, 		validate_hasparam},
	{"rcpt to:<", 	"", 	2, 		3, 		validate_rcpt},
	{"data", 		"", 	3, 		4, 		validate_noparam},
	{"",			"",		4,		5,		},
	{"quit", 		"", 	5, 		0,		validate_noparam},
	{""}
};


void create_fromline(){
	time_t t;
	memset(FROMLINE, 0, 100);
	time(&t);
	sprintf(FROMLINE, "From %s %s", sender, ctime(&t));
}

/* Gibt FD für das Anhängen an Mailbox zurück und schreibt From Separator Line in die Mailbox */
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
	char error[LINEMAX] = "500 ";
	char filepath[MAXDATEIPFAD] = {0};
	int mailbox_fd;
	int state = 0;
	DBRecord db_record = {"", "", ""};
	int db_index;
	
	LineBuffer *linebuf = NULL;
	int umbruch;
	
	char response[LINEBUFFERSIZE] = {0};
	
	clientsocket = infd;
	
	signal(SIGINT, smtp_sighandler);
	
	write(outfd, "220 meinsmtpserver.de\r\n", strlen("220 meinsmtpserver.de\r\n")+1);
	
	
	while(1){
		/* RESETTE LINE */
		memset(line, 0, LINEMAX);
		
		
		/* Initialisiere LineBuffer falls erforderlich */
		if (linebuf == NULL) {
			linebuf = buf_new(infd, "\r\n");
		}
		
		if (buf_readline(linebuf, line, LINEBUFFERSIZE) != -1) {
			prolRes = processLine(line, state, SMTP_DIALOG);
		}
		
		if (prolRes.dialogrec != NULL) {
			if (prolRes.failed) {
				sprintf(error, "550 prolRes failed\r\n");
			}
		} else {
			if (state != 4) {
				sprintf(response, "502 Command not implemented\r\n");
				if (write(outfd, response, strlen(response))<0 ) {
					perror("Server Antwort");
				}
				continue;
			} else {
				strcat(line,"\n"); /* Fügt \n an Datei wenn gerade in DATACONTENT angekommen. */
			}
		}

		/* DATA CONTENT*/
		if(state == 4){
			if(mailbox_fd < 0){
				perror("Bei Oeffnen der mbox-Datei");
				exit(2);
			}
			
			while((umbruch = buf_readline(linebuf, line, LINEBUFFERSIZE)) != -1){		
				if(!strncmp(line, ".",1)){
					if (write(mailbox_fd, "\n\n", 1)< 0) {
						perror("MailEnde schreiben");
					}
					close(mailbox_fd);
					
					sprintf(response, "250 Ok\r\n");					
					state++;
					break;
				} 
				strcat(line, "\n");
				if (write(mailbox_fd, line, strlen(line)) <0) {
					perror("Zeile anhängen");
				}
				memset(line, 0, LINEMAX);
			}
		} /* DATA END */
		
		if (prolRes.dialogrec != NULL) {	
			/* HELO */
			if(!strncasecmp(prolRes.dialogrec->command, "helo",4)){
				if(!prolRes.failed){
					state = prolRes.dialogrec->nextstate;
					sprintf(response, "250 Ok\r\n");
				} else {
					sprintf(response, "%s prolResult failed or NULL\r\n",error);

				}
			}
			
			/* MAIL FROM */
			else if(!strncasecmp(prolRes.dialogrec->command, "mail from:<",11)){
				if(!prolRes.failed){
					
					memset(sender, 0, 100);
					strcpy(sender,prolRes.dialogrec->param);
					
					printf("sender %s\n", sender);
					
					if (sender[strlen(sender)-1] == '>') {
						sender[strlen(sender)-1] = '\0';
						sprintf(response, "250 Ok\r\n");
						state = prolRes.dialogrec->nextstate;
					} else {
						sprintf(response, "%s SyntaxError: Expected '>'\r\n", error);
					}				
					
				} else {
					sprintf(response, "%s prolResult failed\r\n", error);
				}
			}
			
			/* RCPT TO */
			else if(!strncasecmp(prolRes.dialogrec->command, "rcpt to:<",9)){
				if(!prolRes.failed){
					strcpy(db_record.key, mailbox);
					
					db_index = db_search("mailserver.db", 0, &db_record);
					if(db_index >= 0){
						db_get("mailserver.db", db_index, &db_record);
					}
					
					strcpy(filepath, db_record.value);
					sprintf(smtp_lockfile, "%s.lock", mailbox);
					
					if(!lock_smtp_fp(smtp_lockfile)){
						perror("File locked!");
						sprintf(response, "500 file already locked bei someone else.\r\n");
					} else {
						sprintf(response, "250 Ok\r\n");
						state = prolRes.dialogrec->nextstate;
					}
				} else {
					sprintf(response, "%s %s\r\n",error, prolRes.info);
				}
			}
			
			/* DATA */
			else if(!strncasecmp(prolRes.dialogrec->command, "data",4)){
				if(!prolRes.failed){
					
					
					sprintf(response, "354 End data with <CR><LF>.<CR><LF>\r\n");
					
					create_fromline();
					mailbox_fd = append_to_mailbox(filepath);
					
					if (mailbox_fd <0 ){
						sprintf(response, "%s Oeffnen der Mailbox-Datei", error);
						perror("Oeffnen der Mailbox < 0");
					}
					state = prolRes.dialogrec->nextstate;

				} else {
					sprintf(response, "%s ProlRes\r\n", error);
				}
				
			}
			
			/* QUIT */
			else if(!strncasecmp(prolRes.dialogrec->command, "quit", 4)){
				if(!prolRes.failed){
					state = prolRes.dialogrec->nextstate;
					
					if (unlock_smtp_fp(smtp_lockfile) == -1) {
						sprintf(response,"%s Could not unlock mailbox\r\n", error);
					} else {
						sprintf(response, "221 Bye\r\n");
					}
					
					
				} else {
					sprintf(response, "%s ProlRes failed or dialogrec==null \r\n", error);
				}
				
				if (write(outfd, response, strlen(response)) <0) {
					perror("Write");
				}		
				break;
			}
		}
		
		if (write(outfd, response, strlen(response))<0) {
			perror("Write");
		}
		
		sprintf(error, "500 "); /* Reset Error Code */
		
	}
	
	/* Aufräumen am Ende */
	if (linebuf != NULL) {
		buf_dispose(linebuf);
	}
	
	return 0;
}

#include "pop3.h"
#include "smtp.h"
#include "database.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <ifaddrs.h>
#include <pthread.h>




const char *LOCALHOST = "127.0.0.1";
const char *STD_POP3_PORT = "8110";
const char *STD_SMTP_PORT = "8025";
const char *mail_server_filepath = "mailserver.db";

int clientsock, popsock, smtpsock;

void mail_sighandler_father(int sig) {
	close(popsock);
	signal(SIGINT, SIG_DFL);
}


void smtp_sighandler_father(int sig) {
	close(smtpsock);
	signal(SIGINT, SIG_DFL);
}

void *smtp_request(void *func) {
	process_smtp(clientsock, clientsock);
	close(clientsock);
	pthread_exit(NULL);
}

int main(void) {
	int running=1, pid, port, host;
	struct sockaddr_in servaddr, clientaddr;
	fd_set readfds, writefds;
	int nfds;
	pthread_t thread_id;
	socklen_t clientlen;
	int sel_result;
	
	DBRecord pop3_portrec = {"port", "pop3", ""};
	DBRecord pop3_hostrec = {"host", "pop3", ""};
	DBRecord smtp_portrec = {"port", "smtp", ""};
	DBRecord smtp_hostrec = {"host", "smtp", ""};
	
	/* POP 3 Setup */
	/* Get Port */
	port = db_search(mail_server_filepath, 0, &pop3_portrec);
	if (port > 0) {
		db_get(mail_server_filepath, port, &pop3_portrec);
	} else {
		strcpy(pop3_portrec.value, STD_POP3_PORT);
	}
	port = atoi(pop3_portrec.value);
	
	/* Get Host */
	host = db_search(mail_server_filepath, 0, &pop3_hostrec);
	
	if (port > 0) {
		db_get(mail_server_filepath, port, &pop3_portrec);
	} else {
		strcpy(pop3_hostrec.value, LOCALHOST);
	}
	
	printf("POP3-Settings: %s %s\n", pop3_hostrec.value, pop3_portrec.value);
	
	/* create Socket */
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if ((popsock = socket(AF_INET, SOCK_STREAM, 0)) <0 ) {
		perror("socket"); exit(-1);
	}
	
	if (setsockopt(popsock, SOL_SOCKET, SO_REUSEADDR, &running, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}
	
	if (bind(popsock, (struct sockaddr*) &servaddr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind"); exit(-1);
	}
	
	if (listen(popsock, 5) < 0) {
		perror("listen");
		exit(-1);
	}
	
	
	/* SMTP Setup */
	port = db_search(mail_server_filepath, 0, &smtp_portrec);
	if(port >= 0){
		db_get(mail_server_filepath, port, &smtp_portrec);
	} else {
		strcpy(smtp_portrec.value, STD_SMTP_PORT);
	}
	
	port = atoi(smtp_portrec.value);
	
	host = db_search(mail_server_filepath, 0, &smtp_hostrec);
	if(host >= 0){
		db_get(mail_server_filepath, host, &smtp_hostrec);
	} else {
		strcpy(smtp_hostrec.value, LOCALHOST);
	}
	
	printf("SMTP-Settings: %s %s\n", smtp_hostrec.value, smtp_portrec.value);

	
	servaddr.sin_family = AF_INET;
	/*servaddr.sin_addr.s_addr = htonl(INADDR_ANY);*/
	servaddr.sin_addr.s_addr = inet_addr(smtp_hostrec.value);
	servaddr.sin_port = htons(port);
	
	if ((smtpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(-1);
	}
	
	if (setsockopt(smtpsock, SOL_SOCKET, SO_REUSEADDR, &running, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}
	
	if (bind(smtpsock, (struct sockaddr*) &servaddr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		exit(-1);
	}
	
	if (listen(smtpsock, 5) < 0) {
		perror("listen");
		exit(-1);
	}
	
	printf("Server running...\n");
	
	
	for(;;) {
		
		/* RESET FD SETS */
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(0, &readfds);
		FD_SET(popsock, &readfds);
		FD_SET(smtpsock, &readfds);
		
		if(popsock < smtpsock) {
			nfds = smtpsock;
		} else {
			nfds = popsock;
		}
		
		nfds++;
		

		if ((sel_result = select(nfds, &readfds, &writefds, NULL, NULL)) <0) {
			perror("Fehler bei select()"); exit(1);
		}
		
		
		/* POP3 Handling */
		if (FD_ISSET(popsock, &readfds) ) {
			/* Input von popsock verfügbar */
			clientlen = (socklen_t)sizeof(struct sockaddr);
			clientsock = accept(popsock, (struct sockaddr *)&clientaddr, &clientlen);
			
			if (clientsock < 0){
				perror("accept");exit(-1);
			}
			
			pid = fork();
			if (pid == -1) {
				perror("Fehler bei fork()");
			} else if (pid == 0){
				/* Sohn Prozess */
				close(popsock);
				process_pop3(clientsock, clientsock);
				close(clientsock);
			} else {
				/* Vater Process */
				signal(SIGINT, mail_sighandler_father);
				close(clientsock);
				continue;
			}
		}
		

		/* SMTP Handling */
		if(FD_ISSET(smtpsock, &readfds)){
			/* Input von Deskriptor smtptsock verfügbar */
			clientlen = (socklen_t) sizeof(struct sockaddr);
			clientsock = accept(smtpsock, (struct sockaddr *) &clientaddr, &clientlen);
			
			if (clientsock < 0){
				perror("accept");
				exit(-1);
			}
			
			pthread_create(&thread_id, NULL, smtp_request, NULL);
			pthread_detach(thread_id);
			
		}
		
	
	}
	
	
	return 0;
}

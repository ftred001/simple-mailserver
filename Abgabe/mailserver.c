#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
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

#include "database.h"
#include "pop3.h"
#include "smtp.h"

char[] STD_PORT = "8110";
char[] LOCALHOST = "127.0.0.1";

int clientsock, popsock, smtpsock;

void mail_sighandler_father(int sig) {
	close(popsock);
	signal(SIGINT, SIG_DFL);
}

void stmp_sighandler_father(int sig) {
	close(smtpsock);
	signal(SIGINT, SIG_DFL);
}

void *smtp_request(void *func) {
	process_smtp(clientsock, clientsock);
	close(clientsock);
	pthread_exit(NULL);
}

int main(void) {
	int clientlen, running=1, pid, port, host;
	struct sockaddr_in servaddr, clientaddr;
	fd_set readfds, writefds;
	int maxfd;
	pthread_t thread_id;
	
	DBRecord pop3_portrec = {"port", "pop3", ""};
	DBRecord pop3_hostrec = {"host", "pop3", ""};
	DBRecord smtp_portrec = {"port", "smtp", ""};
	DBRecord smtp_hostrec = {"host", "smtp", ""};
	
	/* POP 3 */
	/* Get Port */
	port = db_search("mailserver.db", 0, &pop3_portrec);
	if (port > 0) {
		db_get("mailserver.db", port, &smtp_portrec);
	} else {
		strcpy(pop3_portrec.value, STD_PORT);
	}
	port = atoi(pop3_portrec.value);
	
	/* Get Host */
	host = db_search("mailserver.db", 0, &pop3_hostrec);
	
	if (port > 0) {
		db_get("mailserver.db", port, &host_portrec);
	} else {
		strcpy(pop3_hostrec.value, LOCALHOST);
	}
	
	/* create Socket */
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if ((popsock = socket(AF_INET, SOCK_STREAM, 0)) <0 ) {
		perror("socket"); exit(-1);
	}
	
	if (setsockopt(popsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}
	
	if (bind(popsock, (struct sockaddr*) &servaddr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind"); exit(-1);
	}
	
	if (listen(popsock, 5) < 0) {
		perror("listen");
		exit(-1);
	}
	
	
	
	/* SMTP */
	
	
	/* */
	
	
	return 0;
}

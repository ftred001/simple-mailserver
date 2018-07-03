#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "database.h"
#include "pop3.h"

#define PORTNUMMER 1234

/* Sockets: Immer bidirektional!
 * 1) Socket erstollen mit socket(int domain, int type, int protocol)
 * 2) Zu einer IP und Port connecten: connect()
 * 
 * */


DBRecord *get_portDBR(int port) {
	DBRecord *dbrec = (DBRecord*)calloc(1, sizeof(DBRecord));
	char *port_string = "1234"; 
	
	
	strcpy(dbrec->key,"port");
	strcpy(dbrec->cat,"pop3"); 

	
	if (port>1024 && port < 65535) {
		sprintf(port_string, "%d", port);
	}
	
	strcpy(dbrec->value, port_string);
	
	return dbrec;
}

DBRecord *get_ipDBR(char *ip_adress) {
	DBRecord *dbrec = (DBRecord*)calloc(1, sizeof(DBRecord));
	strcpy(dbrec->key,"host");
	strcpy(dbrec->cat,"pop3");
	strcpy(dbrec->value, ip_adress);
	
	return dbrec;
}

int conn_made(int clientsockfd) {
	ssize_t size;
	char *nachricht = (char*)calloc(1024, sizeof(char));
	
	size = recv(clientsockfd, nachricht, 1024, 0);
	if (size <0) {
		perror("recv");
		return -1;
	}
	
	printf("Received %ld bytes...\n", size);
	
	printf("%s\n", nachricht);

	
	sprintf(nachricht, "%ld\r\n", size+2);
	write(clientsockfd, nachricht, size+2);
	return 0;
}

int main(void) {
	int sockfd, newsockfd, clientlen;
	struct sockaddr_in servaddr, clientaddr;
	
	
	printf("Starting Mailserver...\n");
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORTNUMMER);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket"); exit(-1);
	}
	
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) <0) {
		perror("bind"); exit(-1);
	}
	
	if (listen(sockfd,5)<0) { perror("listen"); exit(-1);}
	
	printf("Waiting for connections!\n");
	
	for (;;) {
		clientlen = sizeof(struct sockaddr);
		newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &clientlen);
		
		if (newsockfd <0) { perror("accept"); exit(-1);}
		
		conn_made(newsockfd);
		
		close(newsockfd);
	}
	
	return 0;
}

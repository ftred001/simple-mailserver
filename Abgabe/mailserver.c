#include "pop3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "database.h"
#include "pop3.h"

#define DEF_PORT "8110"

int listenfd;
struct in_addr *inp;

DBRecord *get_portDBR(char *port) {
	DBRecord *dbrec = (DBRecord*)calloc(1, sizeof(DBRecord));
	int value = atoi(port);
	
	strcpy(dbrec->key,"port");
	strcpy(dbrec->cat,"pop3"); 
	strcpy(dbrec->value, port);
	
	if (value<=1024 || value > 65535) {
		strcpy(dbrec->value, DEF_PORT);
	}
	
	return dbrec;
}

DBRecord *get_ipDBR(char *ip_adress) {
	DBRecord *dbrec = (DBRecord*)calloc(1, sizeof(DBRecord));
	strcpy(dbrec->key,"host");
	strcpy(dbrec->cat,"pop3");
	strcpy(dbrec->value, ip_adress);
	
	return dbrec;
}

void startServer(char* port) {
	struct addrinfo hints, *res, *p;
	
	/* getaddrinfo for hosts */;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo(NULL,port &hints, &res) != 0) {
		perror("gettaddrinfo() error");
		exit(1);
	}
	
	for (p = res; p!=NULL; p=p->ai_next) {
		listenfd = socket(p->ai_family; p->ai_socktype, 0);
		if (listenfd == -1) {
			continue;
		}
		
		if (bind(listenfd, p->addr, p->ai_addrlen) == 0) {
			break;
		}
		
		if (p==NULL) {
			perror("socket() or bind()");
			exit(1);
		}
		
		freeaddrinfo(res);
		
		if (listen(listenfd, 1000000) != 0) {
			perror("listen() error");
			exit(1);
		}
		
	}
	
	
}

int main(int argc, char* argv[]) {
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;
	
	/* Default Values */
	char PORT[6] = DEF_PORT;
	
	startServer("1337");

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORTNUMMER 1234

int main(void) {
	char nachricht[80];
	int zaehl= 0, sockfd, newsockfd, clientlen;
	struct sockaddr_in servaddr, clientaddr;
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORTNUMMER);
	
	printf("%d %d\n",servaddr.sin_addr.s_addr, servaddr.sin_port);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket"); exit(-1);
	}
	
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) <0) {
		perror("bind"); exit(-1);
	}
	
	if (listen(sockfd,5)<0) { perror("listen"); exit(-1);}
	
	for (;;) {
		clientlen = sizeof(struct sockaddr);
		newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
		
		if (newsockfd <0) { perror("accept"); exit(-1);}
		
		sprintf(nachricht, "%d\r\n", ++zaehl);
		write(newsockfd, nachricht, strlen(nachricht));
		close(newsockfd);
	}
	
	return 0;
}


/* Sockets: Immer bidirektional!
 * 1) Socket erstollen mit socket(int domain, int type, int protocol)
 * 2) Zu einer IP und Port connecten: connect()
 * 
 * */

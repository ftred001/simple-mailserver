#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORTNUMMER 1234

int main(int argc, char *argv[]) {
	char buffer[80];
	int wert, sockfd, n;
	struct sockaddr_in servaddr;
	char *host = argv[1];
	char *nachricht = argv[2];
	
	if (inet_aton(host, &servaddr.sin_addr)==0) {
		perror("inet_aton"); exit(1);
	}
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORTNUMMER);
	
	printf("PORT: %d\n", servaddr.sin_port);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0) {
		perror("socket"); exit(-1);
	}
	
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) <0) {
		perror("connect"); exit(-1);
	}
	
	strcat(nachricht, "\r\n");
	
	write(sockfd, nachricht, strlen(nachricht));
	
	n = read(sockfd, buffer, sizeof(buffer)+1);
	sscanf(buffer, "%d", &wert);
	printf("Empfangene Zahl: %d\n", wert);
	
	close(sockfd);
	
	return 0;
}

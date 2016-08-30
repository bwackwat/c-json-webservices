#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024

int main(int argc, char** argv){
	int sockfd, portno, n;
	FILE* fp;
	size_t len = 0;
	struct sockaddr_in serveraddr;
	struct hostent* server;
	char* hostname;
	char buf[BUFSIZE];

	if(argc != 3){
		printf("usage: %s <hostname> <port>\n", argv[0]);
		return 0;
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		printf("socket error\n");
		return 0;
	}

	server = gethostbyname(hostname);
	if(server == 0){
		printf("gethostname failed\n");
		return 0;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	/* bzero((char*)&serveraddr, sizeof(serveraddr)); */
	serveraddr.sin_family = AF_INET;
	memmove(server->h_addr, &serveraddr.sin_addr.s_addr, server->h_length);
	/* bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, server->h_length); */
	serveraddr.sin_port = htons(portno);

	if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1){
		printf("connect error\n");
		return 0;
	}

	if((fp = fopen("client-inputs.txt", "r")) == 0){
		printf("Could not open inputs file!\n");
		return 0;
	}

	printf("-----------------------TESTS STARTING\n");

	while(fgets(buf, BUFSIZE, fp)){
		len = strlen(buf);
		buf[len - 1] = 0;
		if((n = write(sockfd, buf, len)) == -1){
			printf("Write error...\n");
			break;
		}
		printf("-> %s\n", buf);
		memset(buf, 0, BUFSIZE);
		if((n = read(sockfd, buf, BUFSIZE)) == -1){
			printf("Read error...\n");
			break;
		}
		buf[n] = 0;
		printf("<- %s\n", buf);
	}

	printf("-----------------------TESTS COMPLETE!\n");

	fclose(fp);
	close(sockfd);
	return 0;
}

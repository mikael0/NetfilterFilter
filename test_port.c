#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define HOST "127.0.0.1"

int main(int argc, char**argv)
{
	int connfd, sockfd, listenfd,n, clilen;
	struct sockaddr_in servaddr,cliaddr;


	long long i;

	listenfd=socket(AF_INET,SOCK_STREAM,0);

	long buffer_size = 128 * 1024 * 1024 ;

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(12345);
	bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	listen(listenfd,1024);

	sockfd=socket(AF_INET,SOCK_STREAM,0);

	servaddr.sin_addr.s_addr=inet_addr(HOST);
	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	printf("before accept\n");

	clilen=sizeof(cliaddr);
	connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);

	printf("connected\n");

	getchar();

	char* c = (char*)malloc(1024);
	char* j = (char*)malloc(1024);
	int len;
	clock_t start = clock();
	for (i = 0; i < 10000; i++)
	{
		if (send(sockfd, c, 1024, 0) < 0)
			perror("send");
		if ((len = recv(connfd, j, 1024, 0)) < 0) 
			perror("recv");
	}

	clock_t finish = clock();

	free(c);
	free(j);
	printf("Time: %lu\n Bps: %lu\n", (finish - start), (1024*1024 / (finish - start)) * CLOCKS_PER_SEC );

}

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


	int i;

	listenfd=socket(AF_INET,SOCK_DGRAM,0);

	int buffer_size = 65536;
	setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(12345);
	bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));


	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	bzero(&cliaddr, sizeof(cliaddr));
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(HOST);
	servaddr.sin_port=htons(12345);


	printf("before accept\n");

	clilen=sizeof(cliaddr);

	printf("connected\n");

	getchar();

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	}


	int j;
	clock_t start = clock();
	for (i = 0; i < 1024; i++)
	{
		if (sendto(sockfd, &i,sizeof(int), 0, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
			perror("send");
//		printf("%i: sent \n", i );
		if ((recvfrom(listenfd, &j, sizeof(int), 0,(struct sockaddr*) &cliaddr, &clilen )) < 0) 
			perror("recv");
//		printf("%i: received %i\n", i, j);
	
	}

/*	
	   for (i = 0; i < 1024; i++)
	   {
	   if (recvfrom(listenfd, j, 1024, 0, (struct sockaddr*) &cliaddr, &clilen ) < 0) 
	   perror("recvfrom");

	   printf("%i: received %i\n", i, *((int*)j));
	   }	
*/	 
	clock_t finish = clock();

	printf("Time: %lu\n Bps: %lu\n", (finish - start), (64 * 1024*1024 / (finish - start)) * CLOCKS_PER_SEC );

}

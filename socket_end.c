#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "netfilterRedirect.h"


int main(int argc, char* argv[]) {

	int s  ;
	socklen_t len;
	struct sockaddr_un  remote, server;
	char str[MAX_PACKET_SIZE];

	if ((s = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	long done, n;
	printf("Waiting for a connection...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (bind(s, (struct sockaddr *)&remote, len) == -1) {
		perror("bind");
		exit(1);
	}
	/*if (connect(s, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}*/


	printf("Connected.\n");
	done = 0;
	int verdict = ACCEPT;
	do {
		len = sizeof(struct sockaddr_un);
		n = recvfrom(s, str, sizeof(package_with_header), 0, (struct sockaddr*) &server, &len );
		if (n <= 0) {
			if (n < 0) perror("recv");
		}
//		verdict = !verdict;
		printf("%i\n", ((package_with_header*)str)->packet_id);
		verdict_msg resp = {((package_with_header*)str)->packet_id, verdict};	

		if (sendto(s, &resp, sizeof(resp), 0,(struct sockaddr*) &server, len) < 0) {
			perror("send");
		}


	} while (!done);


	close(s);

	return 0;
	}



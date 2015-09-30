#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/netlink.h>

#include "netfilterRedirect.h"


#define NETLINK_NETFILTER 12

typedef struct {
	int fd;
	struct sockaddr_nl local;
	struct sockaddr_nl peer;
	u_int32_t buff_size;
} nl_handle;

int main(int argc, char* argv[]) {


	nl_handle h;
	int s,  len;
	struct sockaddr_un  remote;
	char str[MAX_PACKET_SIZE];


/*nfnl_open()*/
	h.buff_size = 8192;
	if ((h.fd = socket(AF_NETLINK, SOCK_RAW, 12)) == -1) {
		perror("socket");
		exit(1);
	}
	h.local.nl_family = AF_NETLINK;
	h.peer.nl_family = AF_NETLINK;
	
	socklen_t addr_len = sizeof(h.local);
	if (getsockname(h.fd, (struct sockaddr *)&h.local, &addr_len) == -1) {
		perror("getsockname");
		exit(1);
	}

	if (addr_len != sizeof(h.local)) {
		errno = EINVAL;
		exit(1);
	}
/*end*/

	
		

	long done, n;
	printf("Waiting for a connection...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}


	printf("Connected.\n");
	done = 0;
	do {
		n = recv(s, str, sizeof(package_with_header), 0);
		if (n <= 0) {
			if (n < 0) perror("recv");
		}
		verdict_msg resp = {((package_with_header*)str)->packet_id, 1};	

		if (send(s, &resp, sizeof(resp), 0) < 0) {
			perror("send");
		}


	} while (!done);


	close(s);

	return 0;
}



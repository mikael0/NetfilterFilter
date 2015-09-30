#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <errno.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "netfilterRedirect.h"

#define SOCK_PATH_S "serverSock"

static int redirect_pkt(struct nfq_data* tb, int sockdes)
{
	int id = 0;
	int len;
	unsigned char* data;
	struct nfqnl_msg_packet_hdr *ph;

	package_with_header curr_package;

	
	ph = nfq_get_msg_packet_hdr(tb);
	if (ph) {
		id = ntohl(ph->packet_id);
	}

		
	len = nfq_get_payload(tb, &data);
	if (len < 0) {
		perror("payload");
		exit(1);
	}
	
	curr_package.packet_id = id;
	printf("%i\n", id);
	memcpy(curr_package.data, data, len); 
	if (send(sockdes, &curr_package, sizeof(package_with_header), 0) == -1) {
            perror("send");
            exit(1);
        }
	
	return id;				
	
}


static void set_verdict(struct nfq_q_handle* qh, verdict_msg* msg) {

	nfq_set_verdict(qh, msg->packet_id, msg->verdict, 0, NULL);		
	
}

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
	      struct nfq_data *nfa, void *data)
{
	redirect_pkt(nfa, *((int*)data));
	return 0;
}

int main(int argc, char **argv)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	fd_set sockets;
	int fd;
	int rv;
	char buf[MAX_PACKET_SIZE] __attribute__ ((aligned));


	int s, sock_out, len, t;
	struct sockaddr_un local, remote;

	if ((s = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH_S);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
//	unlink(remote.sun_path);
	if (connect(s, (struct sockaddr*)&remote, sizeof(remote))) {
		perror("connect");
		exit(1);
	}
/*
	if (listen(s, 5) == -1) {
		perror("listen");
		exit(1);
	}
*/
	
	for(;;) {

		printf("opening library handle\n");
		h = nfq_open();
		if (!h) {
			fprintf(stderr, "error during nfq_open()\n");
			exit(1);
		}

		printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
		if (nfq_unbind_pf(h, AF_INET) < 0) {
			fprintf(stderr, "error during nfq_unbind_pf()\n");
			exit(1);
		}

		printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
		if (nfq_bind_pf(h, AF_INET) < 0) {
			fprintf(stderr, "error during nfq_bind_pf()\n");
			exit(1);
		}

		printf("binding this socket to queue '0'\n");
		qh = nfq_create_queue(h,  0, &cb, &s);
		if (!qh) {
			fprintf(stderr, "error during nfq_create_queue()\n");
			exit(1);
		}

		printf("setting copy_packet mode\n");
		if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
			fprintf(stderr, "can't set packet_copy mode\n");
			exit(1);
		}


		if (nfq_set_queue_maxlen(qh, 1024) < 0) {
			perror("length");
		}
		
		fd = nfq_fd(h);


		t = sizeof(remote);
/*		if ((sock_out = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
			perror("accept");
			exit(1);
		}
*/
		for (;;) {
			int nready;
			int i;

			FD_ZERO(&sockets);
			FD_SET(fd, &sockets);
			FD_SET(s, &sockets);
			int maxfd = MAX(fd, s);
			nready = select(maxfd + 1, &sockets, NULL, NULL, NULL);
			if (nready == -1) {
				perror("select");
				exit(1);
			}

			for(i=maxfd; i>= 0 && nready>0; i--)
			{
				if(FD_ISSET(i, &sockets))
				{
					nready--;

					if (i == fd) {

						if ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {
							nfq_handle_packet(h, buf, rv);
							continue;
						}

						/* if your application is too slow to digest the packets that
						 * are sent from kernel-space, the socket buffer that we use
						 * to enqueue packets may fill up returning ENOBUFS. Depending
						 * on your application, this error may be ignored. Please, see
						 * the doxygen documentation of this library on how to improve
						 * this situation.
						 */
						if (rv < 0 && errno == ENOBUFS) {
							printf("losing packets!\n");
							continue;
						}
						perror("recv failed");
						break;
					}
					if (i == s) {

						if ((rv = recv(s, buf, sizeof(buf), 0)) >= 0) {

							set_verdict(qh, (verdict_msg*)buf);
							continue;
						}

					}
				}

			}
		}

	}
	printf("unbinding from queue 0\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	close(s);

	exit(0);
}

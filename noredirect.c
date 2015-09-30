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


static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
	      struct nfq_data *nfa, void *data)
{
	static int verdict = 1;
	struct nfqnl_msg_packet_hdr *ph;
	u_int32_t id = 0;
	ph = nfq_get_msg_packet_hdr(nfa);
	if (ph) {
		id = ntohl(ph->packet_id);
	}

//	verdict = !verdict;
	nfq_set_verdict(qh, id, verdict, 0, NULL);
	printf("entering callback\n");
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

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(s, 5) == -1) {
		perror("listen");
		exit(1);
	}

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
		qh = nfq_create_queue(h,  0, &cb, &sock_out);
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

		for (;;) {
			int nready;
			int i;

			FD_ZERO(&sockets);
			FD_SET(fd, &sockets);
			FD_SET(sock_out, &sockets);
			int maxfd = MAX(fd, sock_out);
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
							printf("pkt received\n");
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
					if (i == sock_out) {

						if ((rv = recv(sock_out, buf, sizeof(buf), 0)) >= 0) {
							printf("pkt received from serv\n");

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

	close(sock_out);

	exit(0);
}

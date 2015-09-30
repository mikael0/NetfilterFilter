#ifndef NETFILTER_REDIRECT_H
#define NETFILTER_REDIRECT_H

#include <sys/types.h>

#define MAX_PACKET_SIZE 65536
#define SOCK_PATH "nf_to_sock"

typedef struct {
	u_int32_t packet_id;
	unsigned char data[MAX_PACKET_SIZE];
} package_with_header;

typedef struct {
	u_int32_t packet_id;
	u_int32_t verdict;
} verdict_msg;

typedef enum {
	DROP,
	ACCEPT,
	STOLEN,
	QUEUE,
	REPEAT,
	STOP
} verdict_t;


#endif

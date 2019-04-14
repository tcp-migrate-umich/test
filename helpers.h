#include <stdio.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>
//#include <netinet/tcp.h>
#include <linux/tcp.h>
#include <stdlib.h>
#include <signal.h>

#define SOL_TCP 6

static int the_sock;

void intHandler(int dummy) {
	close(the_sock);
	exit(0);
}

int close_on_kill(int sock) {
	the_sock = sock;
	int err = 0;
	err = signal(SIGINT, intHandler) == SIG_ERR;
	err |= signal(SIGTERM, intHandler) == SIG_ERR;
	return err;
}

int repair_on(int sock) {
	int val = 1;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not put sock into repair mode");
		return -1;
	}
	return 0;
}

int repair_off(int sock) {
	int val = 0;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not take sock out of repair mode");
		return -1;
	}
	return 0;
}

int set_seqs(int sock, int snd_seq, int rcv_seq) {
	// Assumes sock in repair mode
	int queue = TCP_SEND_QUEUE;
	int seq = snd_seq;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR_QUEUE, &queue, sizeof(queue))) {
		perror("could not set repair queue to send");
		return -1;
	}
	if (setsockopt(sock, SOL_TCP, TCP_QUEUE_SEQ, &seq, sizeof(seq))) {
		perror("could not set seqno of send queue");
		return -1;
	}

	queue = TCP_RECV_QUEUE;
	seq = rcv_seq;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR_QUEUE, &queue, sizeof(queue))) {
		perror("could not set repair queue to recv");
		return -1;
	}
	if (setsockopt(sock, SOL_TCP, TCP_QUEUE_SEQ, &seq, sizeof(seq))) {
		perror("could not set seqno of recv queue");
		return -1;
	}
}

int is_migrate_enabled(int sock, bool *enabled) {
	socklen_t len = sizeof(*enabled);
	if (getsockopt(sock, SOL_TCP, TCP_MIGRATE_ENABLED, enabled, &len)) {
		perror("could not get TCP_MIGRATE_ENABLED sockopt");
		return -1;
	}
	return 0;
}

int set_migrate_enabled(int sock, bool enabled) {
	int val = enabled;
	if (setsockopt(sock, SOL_TCP, TCP_MIGRATE_ENABLED, &val, sizeof(val))) {
		perror("could not set TCP_MIGRATE_ENABLED sockopt");
		return -1;
	}
	return 0;
}

int __set_migrate_token(int sock, int token) {
	if (setsockopt(sock, SOL_TCP, TCP_MIGRATE_TOKEN, &token, sizeof(token))) {
		perror("could not set migrate token of sock");
		return -1;
	}
	return 0;
}

int set_migrate_token(int sock, int token) {
	if (repair_on(sock))
		return -1;
	if (__set_migrate_token(sock, token))
		return -1;
	if (repair_off(sock))
		return -1;
	return 0;
}

int get_migrate_token(int sock, int *token) {
	socklen_t len = sizeof(*token);
	if (repair_on(sock))
		return -1;
	if (getsockopt(sock, SOL_TCP, TCP_MIGRATE_TOKEN, token, &len)) {
		perror("could not get migrate token of sock");
		return -1;
	}
	if (repair_off(sock))
		return -1;
	return 0;
}

int check_tcp_fastopen(int sock) {
	puts("Checking socket fastopen options");
	int val;
	socklen_t len = sizeof(val);
	
	if (getsockopt(sock, SOL_TCP, TCP_FASTOPEN, &val, &len)) {
		perror("fail to get TCP_FASTOPEN");
		return -1;
	}
	printf("TCP_FASTOPEN: %i\n", val);

	if (getsockopt(sock, SOL_TCP, TCP_FASTOPEN_CONNECT, &val, &len)) {
		perror("fail to get TCP_FASTOPEN_CONNECT");
		return -1;
	}
	printf("TCP_FASTOPEN_CONNECT: %i\n", val);

	if (getsockopt(sock, SOL_TCP, TCP_FASTOPEN_NO_COOKIE, &val, &len)) {
		perror("fail to get TCP_FASTOPEN_NO_COOKIE");
		return -1;
	}
	printf("TCP_FASTOPEN_NO_COOKIE: %i\n", val);

	return 0;
}

int send_migrate_request(int sock) {
	return system("cat /proc/net/tcp_mig_req_send");
}


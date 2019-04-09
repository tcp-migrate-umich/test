#include <stdio.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>
//#include <netinet/tcp.h>
#include <linux/tcp.h>

#define SOL_TCP 6

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

int set_migrate_token(int sock, int token) {
	int val = 1;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not put sock into repair mode");
		return -1;
	}
	if (setsockopt(sock, SOL_TCP, TCP_MIGRATE_TOKEN, &token, sizeof(token))) {
		perror("could not set migrate token of sock");
		return -1;
	}
	val = 0;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not take sock out of repair mode");
		return -1;
	}
	return 0;
}

int get_migrate_token(int sock, int *token) {
	int val = 1;
	socklen_t len = sizeof(*token);
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not put sock into repair mode");
		return -1;
	}
	if (getsockopt(sock, SOL_TCP, TCP_MIGRATE_TOKEN, token, &len)) {
		perror("could not get migrate token of sock");
		return -1;
	}
	val = 0;
	if (setsockopt(sock, SOL_TCP, TCP_REPAIR, &val, sizeof(val))) {
		perror("could not take sock out of repair mode");
		return -1;
	}
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


#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

/**
 * TCP Uses 2 types of sockets, the connection socket and the listen socket.
 * The Goal is to separate the connection phase from the data exchange phase.
 * */

int main(int argc, char *argv[]) {
	// port to start the server on
	int SERVER_PORT = 8877;

	if (argc > 1) {
		SERVER_PORT = atoi(argv[1]);
	}

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	// create a TCP socket, creation returns -1 on failure int listen_sock;
	int listen_sock; 
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create listen socket\n");
		return 1;
	}

	// check TCP fastopen options (im just curious)
	if (check_tcp_fastopen(listen_sock)) {
		printf("could not check tcp fastopen options on listen socket");
		return 1;
	}
	
	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(listen_sock, wait_size) < 0) {
		printf("could not open socket for listening\n");
		return 1;
	}

	if (close_on_kill(listen_sock)) {
		perror("could not set up inthandler");
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_address;
	int client_address_len = 0;

	// run indefinitely
	fflush(stdout);
	while (true) {
		// open a new socket to transmit data per connection
		int sock;
		if ((sock =
		         accept(listen_sock, (struct sockaddr *)&client_address,
		                &client_address_len)) < 0) {
			printf("could not open a socket to accept data\n");
			return 1;
		}

		int n = 0;
		int len = 0, maxlen = 100;
		char buffer[maxlen];
		memset(buffer, 0, sizeof(buffer));
		char *pbuffer = buffer;

		printf("client connected with ip address: %s\n",
		       inet_ntoa(client_address.sin_addr));
/*
		// Check if connection is migrate enabled,
		// and if so, print out its migrate token
		// (we can't do this immediately after the
		// connect call because of TCP Fastopen;
		// the final ACK in the three-way handshake
		// may not have been received by the time
		// we start recv-ing or send-ing).
		if (is_migrate_enabled(sock, &enabled)) {
			puts("could not check migrate_enabled of connection sock");
			return 1;
		}
		if (enabled) {
			int token = 696969;
			if (get_migrate_token(sock, &token)) {
				puts("could not get token of connection");
				return 1;
			}
			printf("\tconnection has migrate token: %i\n", token);
		} else {
			printf("\tconnection does not permit migration\n");
		}
*/
		if (close_on_kill(sock)) {
			perror("could not set up inthandler");
			return 1;
		}

		// keep running as long as the client keeps the connection open
		while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
			printf("received: '%s'\n", buffer);
			fflush(stdout);

			// echo received content back
			send(sock, buffer, n, 0);

			memset(buffer, 0, sizeof(buffer));
			len = 0;
		}

		puts("closing connection");
		if (close(sock)) {
			perror("could not close socket");
			return 1;
		}
	}

	close(listen_sock);
	return 0;
}

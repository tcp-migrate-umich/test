#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/tcp.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv) {
	const char* server_name = "141.212.110.206";
	int server_port = 8877;
	int num_times = 1;
	int wait_time = 3;
	bool do_forever = false;

	// data that will be sent to the server
	const char* data_to_send = "Gangadhar Hi Shaktimaan hai";

	if (argc <= 1) {
		printf("Usage: ./client [num_times] [server_name] [server_port] [wait_time] [data_to_send]\n");
	}
	if (argc > 1) {
		num_times = atoi(argv[1]);
		if (num_times == -1)
			do_forever = true;
	}
	if (argc > 2) {
		server_name = argv[2];
	}
	if (argc > 3) {
		server_port = atoi(argv[3]);
	}
	if (argc > 4) {
		wait_time = atoi(argv[4]);
	}
	if (argc > 5) {
		data_to_send = argv[5];
	}

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	// htons: port in network order format
	server_address.sin_port = htons(server_port);

	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}
	
	// check TCP fastopen options (im just curious)
	if (check_tcp_fastopen(sock)) {
		printf("could not check tcp fastopen options on client socket");
		return 1;
	}

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		printf("could not connect to server\n");
		return 1;
	}

	if (close_on_kill(sock)) {
		perror("could not set up inthandler");
		return 1;
	}

	char buffer[1000];
	for (int i = 0; i < num_times || do_forever; i++) {
		if (i != 0) {
			sleep(wait_time);
		}

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "[%i] %s", i, data_to_send);

		// send
		send(sock, buffer, strlen(buffer), 0);

		// receive

		int n = 0;
		int len = 0, maxlen = 100;
		char buffer[maxlen];
		memset(buffer, 0, sizeof(buffer));
		char* pbuffer = buffer;

		// will remain open until the server echos back the content
		recv(sock, pbuffer, maxlen, 0);
		printf("received: '%s'\n", buffer);
		/*
		while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
			pbuffer += n;
			maxlen -= n;
			len += n;

			buffer[len] = '\0';
			printf("received: '%s'\n", buffer);
		}
		*/

	}

	// close the socket
	close(sock);
	return 0;
}

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv) {
	
	char *CLIENT_ADDR = "141.212.110.206";
	int CLIENT_PORT = 0;
	int SERVER_PORT = 8877;
	int snd_seq = 0, rcv_seq = 0;

	if (argc <= 4) {
		printf("Usage: ./migrated_server client_addr client_port snd_seq rcv_seq [server_port]\n");
		return 1;
	}

	CLIENT_ADDR = argv[1];
	CLIENT_PORT = atoi(argv[2]);
	snd_seq = atoi(argv[3]);
	rcv_seq = atoi(argv[4]);

	if (argc > 5) {
		SERVER_PORT = atoi(argv[5]);
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	puts("repair on");
	if (repair_on(sock))
		return 1;

	puts("binding sock");
	
	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)))
		return 1;

	printf("connecting socket to client at %s:%i\n", CLIENT_ADDR, CLIENT_PORT);

	struct sockaddr_in client_address;
	memset(&client_address, 0, sizeof(client_address));
	client_address.sin_family = AF_INET;

	// creates binary representation of client name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, CLIENT_ADDR, &client_address.sin_addr);

	// htons: port in network order format
	client_address.sin_port = htons(CLIENT_PORT);

	if (connect(sock, (struct sockaddr *)&client_address, sizeof(client_address)))
		return 1;

	puts("enabling migrate and setting migrate token");
	int token = 1234567;
	if (set_migrate_enabled(sock, true))
		return 1;
	// this will send the migrate request
	if (__set_migrate_token(sock, token))
		return 1;

	printf("Setting snd_seq=%i, rcv_seq=%i\n", snd_seq, rcv_seq);
	if (set_seqs(sock, snd_seq, rcv_seq)) {
		return 1;
	}


	if (close_on_kill(sock)) {
		perror("could not set up inthandler");
		return 1;
	}

	puts("repair off");
	if (repair_off(sock))
		return 1;
	
	// keep running as long as the client keeps the connection open
	int n = 0;
	int len = 0, maxlen = 100;
	char buffer[maxlen];
	memset(buffer, 0, sizeof(buffer));
	char *pbuffer = buffer;
	//while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
	while (true) {
		n = recv(sock, pbuffer, maxlen, 0);

		if (n == 0)
			continue;

		printf("received: '%s'\n", buffer);

		// echo received content back
		send(sock, buffer, n, 0);

		memset(buffer, 0, sizeof(buffer));
		len = 0;
	}

	close(sock);

	return 0;
}

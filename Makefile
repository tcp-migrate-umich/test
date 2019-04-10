
all:
	gcc -o client client.c
	gcc -o server server.c
	gcc -o migrated_server migrated_server.c

CC=gcc --std=gnu99 -g 

all: enc_client enc_server

enc_client.o: enc_client.c enc_client.h
	$(CC) -c enc_client.c

enc_client: enc_client.o
	$(CC) enc_client.o -o enc_client

enc_server.o: enc_server.c enc_server.h
	$(CC) -c enc_server.c

enc_server: enc_server.o
	$(CC) enc_server.o -o enc_server


clean:
	rm -rf enc_client.o enc_client enc_server.o enc_server


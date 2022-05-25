CC=gcc --std=gnu99 -g 

all: enc_client enc_server keygen



enc_client.o: enc_client.c enc_client.h
	$(CC) -c enc_client.c

enc_client: enc_client.o
	$(CC) enc_client.o -o enc_client



enc_server.o: enc_server.c enc_server.h
	$(CC) -c enc_server.c

enc_server: enc_server.o
	$(CC) enc_server.o -o enc_server



keygen.o: keygen.c keygen.h
	$(CC) -c keygen.c

keygen: keygen.o
	$(CC) keygen.o -o keygen



clean:
	rm -rf enc_client.o enc_client enc_server.o enc_server keygen.o keygen


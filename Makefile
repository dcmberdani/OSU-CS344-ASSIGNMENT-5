CC=gcc --std=gnu99 -g 

all: enc_client

enc_client.o: enc_client.c enc_client.h
	$(CC) -c enc_client.c

enc_client: enc_client.o
	$(CC) enc_client.o -o enc_client

clean:
	rm -rf enc_client.o enc_client

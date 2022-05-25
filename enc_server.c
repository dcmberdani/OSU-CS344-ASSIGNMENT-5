//Outline taken directly from lecture
#define IPADDR "127.0.0.1" // Localhost 
#define BUFSIZE 1024
//#define PORT 56124

#include <stdio.h>

//For socket info
#include <sys/types.h>
#include <sys/socket.h>

//Other network stuff
#include <netdb.h>
#include <arpa/inet.h> //inet_pton

//String stuff
#include <string.h>

#include <stdlib.h> //atoi()

#include "enc_server.h"


void testprint() {
	printf("Nothing has crashed and burned yet.\n");
}

int main(int argc, char **argv) {
	//cmd line args
	if (argc != 2){
		printf("ERROR: Incorrect number of parameters\n");
		return -1;
	}
	int PORT = atoi(argv[1]); //Grab port # from cmdline
	printf("Starting server on port: %d\n", PORT);
	
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char *hello = "ENCSERVER: Hello[Server]\n";
	char buffer[BUFSIZE] = {0};

	//From exploration "Communication VIA Sockets"
	socklen_t sizeOfClientIP = sizeof(IPADDR); //All addresses are on localhost

	//Make a socket for the server itself for read/write
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("ENCSERVER: Error: Socket creation error\n");
		return -1;
	}

	//Attach the server socket to the given port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		printf("ENCSERVER: setsocketopt error\n");
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // bind to any address
	address.sin_port = htons(PORT); // format the port #

	//Bind socket to given port
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		printf("ENCSERVER: bind failed\n");
		return -1;
	}

	//Listen out for connections; We need 5 at most
	if (listen(server_fd, 5) < 0) {
		printf("ENCSERVER: listen failed\n");
		return -1;
	}

	//Create new sockets whenever a server is accepted
	while (1) {
		//if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&sizeof(address))) < 0) {
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &sizeOfClientIP)) < 0) {
			printf("ENCSERVER: accepting client failed");
			return -1;
		}
	

		//Perform decryption
		//For now, just print out a test message
		valread = recv(new_socket, buffer, BUFSIZE, 0);
		printf("ENCSERVER: Message Received: %s\n", buffer);
		send(new_socket, hello, strlen(hello), 0);
		printf("ENCSERVER: Message sent\n");

		//EXPERIMENTAL; close server if message "CLOSE" is sent
		if (strcmp(buffer, "CLOSE") == 0)
			break;

		//AFTER EVERYTHING, CLEAR BUFFER
		memset(buffer, '\0', sizeof(buffer));

	}
	
	//Find some way to close the infinite loop? Maybe if the buffer is 0 it'll close

	//testprint();

	return 0;
}

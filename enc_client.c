//Outline taken directly from lecture
#define IPADDR "127.0.0.1" // Localhost 
#define BUFSIZE 1024
#define PORT 56124

#include <stdio.h>

//For socket info
#include <sys/types.h>
#include <sys/socket.h>

//Other network stuff
#include <netdb.h>
#include <arpa/inet.h> //inet_pton

//String stuff
#include <string.h>


void testprint() {
	printf("Nothing has crashed and burned yet.\n");
}

int main() {
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "ENCCLIENT: Hello[Client]";
	char buffer[BUFSIZE] = {0};

	//Make a socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ENCCLIENT: Error: Socket creation error\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT); //Maybe make port a cmd line arg or in a shared header?

	//Convert IP address to usable binary
	//if (inet_pton(AF_INET, "localhost", &serv_addr.sin_addr) <= 0) {
	if (inet_pton(AF_INET, IPADDR, &serv_addr.sin_addr) <= 0) {
		printf("ENCCLIENT: Error: Invalid address, address not supported\n");
		return -1;
	}

	//Actually connect to the server
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("ENCCLIENT: Connection Failed\n");
		return -1;
	}

	//Request Encryption; 
	//For now, just do a test print
	send(sock, hello, strlen(hello), 0);
	printf("ENCCLIENT: Message sent\n");
	//valread = recv(sock, buffer, BUFSIZE);
	valread = recv(sock, buffer, BUFSIZE, 0); //0 specifies no flags
	printf("ENCCLIENT: Message received: %s\n", buffer);

	//testprint();

	return 0;
}

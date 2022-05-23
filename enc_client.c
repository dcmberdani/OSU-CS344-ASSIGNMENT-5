//Outline taken directly from lecture
#define IPADDR 127.0.0.1 // Localhost 
#define BUF_SIZE 1024
#define PORT 56124

#include <stdio.h>

//For socket info
#include <sys/types.h>
#include <sys/socket.h>

//Other network stuff
#include <netdb.h>


void testprint() {
	printf("Nothing has crashed and burned yet.\n");
}

int main() {
	int server_fd, new_socket, valread;
	struct sockaddr_in address;

	testprint();

	return 0;
}

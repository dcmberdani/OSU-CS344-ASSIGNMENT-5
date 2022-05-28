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

#include <semaphore.h> //obv
#include <fcntl.h> // O_CREAT
#include <sys/stat.h> // Mode constants
#include <unistd.h> //fork()
#include <sys/wait.h> //waitpid

#include "enc_server.h"



int main(int argc, char **argv) {
	//cmd line args
	if (argc != 2){
		fprintf(stderr, "ENCSERVER: ERROR: Incorrect number of parameters\n");
		return -1;
	}
	int PORT = atoi(argv[1]); //Grab port # from cmdline
	//printf("ENCSERVER: Starting server on port: %d\n", PORT);
	
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	//char *hello = "ENCSERVER: Hello[Server]\n";
	//char *input = malloc(sizeof(char) * BUFSIZE);
	char *input;
	char *ciphertext;
	char buffer[BUFSIZE] = {0};
	int status;

	//From exploration "Communication VIA Sockets"
	socklen_t sizeOfClientIP = sizeof(IPADDR); //All addresses are on localhost

	//Make a socket for the server itself for read/write
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("ENCSERVER: Error: Socket creation error\n");
		return -1;
	}

	//Attach the server socket to the given port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("ENCSERVER: setsocketopt error\n");
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // bind to any address
	address.sin_port = htons(PORT); // format the port #

	//Bind socket to given port
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("ENCSERVER: ERROR: bind failed\n");
		return -1;
	}

	//Listen out for connections; We need 5 at most
	if (listen(server_fd, 5) < 0) {
		perror("ENCSERVER: ERROR: listen failed\n");
		return -1;
	}

	//int clientCount = 0;
	//https://stackoverflow.com/questions/8359322/how-to-share-semaphores-between-processes-using-shared-memory
	sem_t *semOpenClients = sem_open("semOpenClients", O_CREAT, 0644, 5); //Public semaphore of value 5; Don't fork unless one is open
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	pid_t pid;
	//Create new sockets whenever a server is accepted
	while (1) {
		memset(buffer, '\0', sizeof(buffer));

		//First, fork the server if there are less than 5 total connections

		int stat;
		sem_getvalue(semOpenClients, &stat);
		sem_wait(semOpenClients);




		//if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&sizeof(address))) < 0) {
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &sizeOfClientIP)) < 0) {
			perror("ENCSERVER: ERROR: accepting client failed");
			//return -1;
			continue;
		}
	

		pid = fork();
		
		//Error fork case
		if (pid < 0) {
			perror("ENCSERVER: ERROR: forking failed");
			exit(1);
		}
		//Child fork case; Here is where connection is handled
		if (pid == 0) {
			//First, receive a message
			valread = recv(new_socket, buffer, BUFSIZE, 0);
			//printf("ENCSERVER: Message Received: %s\n", buffer);

			//Now, verify the connection if the message is appropriate
			if (strcmp(buffer, "ENCCLIENT") == 0){
				//printf("ENCSERVER: YOU ARE VERIFIED\n");
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "VERIFIED");
				send(new_socket, buffer, strlen(buffer), 0);
			} else {

				//printf("ENCSERVER: YOU ARE NOT VERIFIED\n");
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "NOT VERIFIED");
				send(new_socket, buffer, strlen(buffer), 0);
				//You must also terminate the connection here as well;
				shutdown(new_socket, SHUT_RDWR);
				//Increment semaphore now that it's closed
				sem_post(semOpenClients);
				continue;
			}
			

			//Perform decryption with the newly received key and plaintext
			memset(buffer, '\0', sizeof(buffer));
			valread = recv(new_socket, buffer, BUFSIZE, 0);
			input = malloc(sizeof(char) * BUFSIZE);
			strcpy(input, buffer);
			ciphertext = encryptText(input);

			//printf("ENCSERVER: Outputted Ciphetext: %s\n", ciphertext);

			//send(new_socket, hello, strlen(hello), 0);
			send(new_socket, ciphertext, strlen(ciphertext), 0);
			//printf("ENCSERVER: Message sent\n");


			//AFTER EVERYTHING, CLEAR BUFFER AND FREE CIPHERTEXT/INPUT
			free(ciphertext);
			free(input);

			sem_post(semOpenClients);
		}
		//Parent process; Basically waiting for children to finish;
		//	I believe this is an infinite loop, waits for children to end;
		//	Source: https://github.com/solomreb/cs344-prog4/blob/master/otp_enc_d.c
		while (pid > 0) {
			pid = waitpid(-1, &status, WNOHANG); //Waitpid -1 means wait for ANY child process
		}


		//EXPERIMENTAL; close server if message "CLOSE" is sent
		if (strstr(buffer, "CLOSE") != NULL)
			break;
	}
	
	//Find some way to close the infinite loop? Maybe if the buffer is 0 it'll close

	//testprint();

	return 0;
}


//This actually does the encryption
char* encryptText(char *input) {

	char *token;
	char plaintext[BUFSIZE] = {0};
	char key[BUFSIZE] = {0};
	char *ciphertext = malloc(sizeof(char) * BUFSIZE);
	memset(ciphertext, '\0', BUFSIZE); //Make sure the output buffer for ciphertext is clear
	int keyInt, ptInt, ctInt;
	//strcpy(ciphertext, "TEST");

	//Now, separate the string into "plaintext" and "key"; They're separated by a '|'
	strcpy(plaintext, strtok_r(input, "|", &input));
	strcpy(key, strtok_r(input, "|", &input));
	//printf("\nPT: %s\n\tK: %s\n", plaintext, key);

	//Now, perform the encryption operation
	//	To do so, ierate through the plaintext string;
	//	Take every char of the plaintext/key and make it an int;
	//	Subtract 65 from the pt int (ASCII 'A') 
	//	Then, add the key, modulo by 26 to prevent invalid chars, and finally add 65 back
	//The result is the encrypted char; So, just add that back into the string
	//SPECIAL CASES: the ' ' or space character
	//	When read in from outside, it's set to be 
	//	When any char is set to 26 + 'A', it becomes a space (ASCII 32)
	for (int i = 0; i < strlen(plaintext); i++) {
		//Set spaces to be 'A + 27'
		if (plaintext[i] == ' ') 
			plaintext[i] = ('A' + 26);
		
		if (key[i] == ' ') 
			key[i] = ('A' + 26);
		
		//printf("CHAR: %c\n", plaintext[i]);

		ptInt = plaintext[i] - 'A';
		keyInt = key[i] - 'A';
		ctInt = ((ptInt + keyInt) % 27) + 'A'; //This is the encrypted char
		//if (ctInt == (27 + 'A'))
		if (ctInt == (26 + 'A'))
			ctInt = 32;
		ciphertext[i] = ctInt;
	}

	return ciphertext;

	
}


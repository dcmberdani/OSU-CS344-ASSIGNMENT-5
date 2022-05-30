//Outline taken directly from lecture
#define IPADDR "127.0.0.1" // Localhost 
#define BUFSIZE 150000
#define S_BUFSIZE 256

#include <stdio.h>

//For socket info
#include <sys/types.h>
#include <sys/socket.h>

//Other network stuff
#include <netdb.h>
#include <arpa/inet.h> //inet_pton

//Other misc. functionality
#include <string.h>
#include <stdlib.h> //atoi()
#include <semaphore.h> //obv
#include <fcntl.h> // O_CREAT
#include <sys/stat.h> // Mode constants
#include <unistd.h> //fork()
#include <sys/wait.h> //waitpid

#include "enc_server.h"



int main(int argc, char **argv) {
	//ID string, same as in enc_client
	char idstring[S_BUFSIZE] = "ENCSERVER";

	//cmd line args
	if (argc != 2){
		fprintf(stderr, "%s: ERROR: Incorrect number of parameters\n", idstring);
		return -1;
	}
	int port = atoi(argv[1]); //Grab port # from cmdline
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char *input;
	char *ciphertext;
	char buffer[BUFSIZE] = {0};
	int status;


	/*
	 * This initial block of code is just setting up the initial server sockets
	 * 	As in the enc_client, essentially all of this was taken from the networking lecture
	 */

	//From exploration "Communication VIA Sockets"
	socklen_t sizeOfClientIP = sizeof(IPADDR); //All addresses are on localhost

	//Make a socket for the server itself for read/write
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		fprintf(stderr, "%s: Error: Socket creation error\n", idstring);
		return -1;
	}

	//Attach the server socket to the given port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		fprintf(stderr, "%s: setsocketopt error\n", idstring);
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // bind to any address
	address.sin_port = htons(port); // format the port #

	//Bind socket to given port
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		fprintf(stderr, "%s: ERROR: bind failed\n", idstring);
		return -1;
	}

	//Listen out for connections; We need 5 at most
	if (listen(server_fd, 5) < 0) {
		fprintf(stderr, "%s: ERROR: listen failed\n", idstring);
		return -1;
	}


	//https://stackoverflow.com/questions/8359322/how-to-share-semaphores-between-processes-using-shared-memory
	//	Basically this is a semaphore that ensures only 5 processes can be used at once
	sem_t *semOpenClients = sem_open("semOpenClients", O_CREAT, 0644, 5); //Public semaphore of value 5; Don't fork unless one is open
	//Temp fix as the semaphore seems to not want to actually work w/a default value
	//	I never found out how to fix this in the actual final version, so I guess this scuffed method is all I have
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);
	sem_post(semOpenClients);

	pid_t pid;
	//Create new sockets whenever a server is accepted
	//	My fork protocol was inspired by the source linked below
	//	https://github.com/solomreb/cs344-prog4/blob/master/otp_enc_d.c
	while (1) {
		memset(buffer, '\0', sizeof(buffer));

		//First, fork the server if there are less than 5 total connections
		int stat;
		sem_getvalue(semOpenClients, &stat);
		sem_wait(semOpenClients);


		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &sizeOfClientIP)) < 0) {
			fprintf(stderr, "%s: ERROR: accepting client failed\n", idstring);
			//return -1;
			continue;
		}
	

		pid = fork();
		
		//Error fork case
		if (pid < 0) {
			fprintf(stderr, "%s: ERROR: forking failed\n", idstring);
			exit(1);
		}
		//Child fork case; Here is where connection is handled
		if (pid == 0) {
			//If verification fails, shut down the socket
			if (verifyClient(new_socket, valread) == 0) {
				shutdown(new_socket, SHUT_RDWR);
				sem_post(semOpenClients);
				continue;
			}
			
		
			/*
			 * This block of code is used to read in packets sent by the client
			 * First, the client will send the number of packets used to contain the plaintext/ciphetext
			 * 	The server will receive this number and store it
			 * Then, the server sends a confirmation message ensuring that the number of packets received is correct
			 * Finally, the client will start sending packets one by one, which the server will receive
			 * Basically, it's a handshake to exchange a packet count, then actually sending packets
			 * When fully received, the server then cleans the input and encrypts it
			 */
			char temp[S_BUFSIZE] = {0};
			valread = recv(new_socket, temp, S_BUFSIZE, 0);
			char *end;
			int packets = (int) strtol(temp, &end, 10);

			//Send confirmation message to send text back
			memset(temp, '\0', S_BUFSIZE);
			strcpy(temp, "go");
			send(new_socket, temp, S_BUFSIZE, 0);
			

			//Now, receive the aforementioned number of packets
			memset(buffer, '\0', sizeof(buffer));
			for (int i = 0; i < packets; i++) {
				memset(temp, '\0', S_BUFSIZE);
				valread = recv(new_socket, temp, S_BUFSIZE, 0);
				strcat(buffer, temp);
			}

			input = malloc(sizeof(char) * BUFSIZE);
			strcpy(input, buffer);

			char* temp15 = input;
			input = cleanTransmittedInput(input);
			free(temp15);


			//Encrypt the ciphertext and store it
			ciphertext = encryptText(input);



			/*
			 * This block of code now sends the ciphertext back to the client using the same protocol as above
			 * This time, the roles are reversed, with the server sending and the client receiving
			 * Since the code is basically identical I won't rehash it here
			 */
			//Now, repeat on the server side
			packets = 0;
			packets = strlen(ciphertext) / S_BUFSIZE;
			if (strlen(ciphertext) - (packets * S_BUFSIZE) > 0) packets +=1;

			memset(temp, '\0', S_BUFSIZE);
			sprintf(temp, "%d\n", packets);
			send(new_socket, temp, S_BUFSIZE, 0);


			memset(temp, '\0', S_BUFSIZE);
			valread = recv(new_socket, temp, S_BUFSIZE, 0); //Wait for the confirmation message from the server
			if (strcmp (temp, "go") != 0) {
				fprintf(stderr, "%s: ERROR: Transmission failed.\n", idstring);
				exit(1);
			}
			
			//Finally, incrementally send out the plaintext

			for (int i = 0; i < packets; i++) {
				memset(temp, '\0', S_BUFSIZE);
				strncpy(temp, ciphertext, S_BUFSIZE);	
				memmove(ciphertext, ciphertext+S_BUFSIZE, BUFSIZE - S_BUFSIZE); //Careful with this precision
				send(new_socket, temp, S_BUFSIZE, 0);
			}


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

	}
	
	return 0;
}


//Check client for futher comments
char* cleanTransmittedInput(char *buffer) {
	char *newbuf = malloc(sizeof(char) * BUFSIZE);
	memset(newbuf, '\0', BUFSIZE);
	char *token;
	char *saveptr;
	while (token = strtok_r(buffer, "", &buffer)) { //Removes all ^B chars from the input
		strcat(newbuf, token);
	}

	return newbuf;

}

//Reads into a small buffer and checks for the correct
//	If it's good, continue; Returns 1
//	If not, then don't; Returns 0
int verifyClient(int new_socket, int valread) {
	char newbuffer[S_BUFSIZE] = {0};
	
	//First, receive a message
	valread = recv(new_socket, newbuffer, S_BUFSIZE, 0);

	//Now, verify the connection if the message is appropriate
	if (strcmp(newbuffer, "ENCCLIENT") == 0){
		memset(newbuffer, '\0', S_BUFSIZE);
		strcpy(newbuffer, "VERIFIED");
		send(new_socket, newbuffer, S_BUFSIZE, 0);

		return 1;
	} else {
		memset(newbuffer, '\0', S_BUFSIZE);
		strcpy(newbuffer, "NOT VERIFIED");
		send(new_socket, newbuffer, S_BUFSIZE, 0);
		//You must also terminate the connection here as well;

		return 0;
	}
		
}


//This actually does the encryption
char* encryptText(char *input) {

	char *token;
	char plaintext[BUFSIZE] = {0};
	char key[BUFSIZE] = {0};
	char *ciphertext = malloc(sizeof(char) * BUFSIZE);
	memset(ciphertext, '\0', BUFSIZE); //Make sure the output buffer for ciphertext is clear
	int keyInt, ptInt, ctInt;

	//Now, separate the string into "plaintext" and "key"; They're separated by a '|'
	strcpy(plaintext, strtok_r(input, "|", &input));
	strcpy(key, strtok_r(input, "|", &input));

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
		
		ptInt = plaintext[i] - 'A';
		keyInt = key[i] - 'A';
		ctInt = ((ptInt + keyInt) % 27) + 'A'; //This is the encrypted char
		if (ctInt == (26 + 'A'))
			ctInt = 32;
		ciphertext[i] = ctInt;
	}

	return ciphertext;

	
}

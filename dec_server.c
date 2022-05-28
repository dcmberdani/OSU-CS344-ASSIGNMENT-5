//Outline taken directly from lecture
#define IPADDR "127.0.0.1" // Localhost 
#define BUFSIZE 150000
//#define BUFSIZE 1024
#define S_BUFSIZE 256
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

#include "dec_server.h"



int main(int argc, char **argv) {
	char idstring[S_BUFSIZE] = "DECSERVER";

	//cmd line args
	if (argc != 2){
		fprintf(stderr, "%s: ERROR: Incorrect number of parameters\n", idstring);
		return -1;
	}
	int port = atoi(argv[1]); //Grab port # from cmdline
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









	//int clientCount = 0;
	//https://stackoverflow.com/questions/8359322/how-to-share-semaphores-between-processes-using-shared-memory
	sem_t *semOpenClients = sem_open("semOpenClients", O_CREAT, 0644, 5); //Public semaphore of value 5; Don't fork unless one is open
	//Temp fix as the semaphore seems to not want to actually work w/a default value
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
			
		


			
			//First, receive the number of packets
			char temp[S_BUFSIZE] = {0};
			valread = recv(new_socket, temp, S_BUFSIZE, 0);
			char *end;
			int packets = (int) strtol(temp, &end, 10);
			//int packets = itoa(temp);
			//fprintf(stdout, "DECSERVER: Received as packet count: %d.\n", packets);

			//Send confirmation message to send text back
			memset(temp, '\0', S_BUFSIZE);
			strcpy(temp, "go");
			send(new_socket, temp, S_BUFSIZE, 0);
			

			//Now, receive the aforementioned number of packets
			memset(buffer, '\0', sizeof(buffer));
			for (int i = 0; i < packets; i++) {
				memset(temp, '\0', S_BUFSIZE);
				valread = recv(new_socket, temp, S_BUFSIZE, 0);
				//fprintf(stdout, "%s: JUST READ: %s\n", idstring,  temp);
				strcat(buffer, temp);
				//printf("DECSERVER: Read packet #: %d\n", i+1);
			}

			//printf("Total Buffer: %s\n", buffer);



			//Perform decryption with the newly received key and plaintext
			//memset(buffer, '\0', sizeof(buffer));
			//valread = recv(new_socket, buffer, BUFSIZE, 0);
			//input = malloc(sizeof(char) * BUFSIZE);
			//strcpy(input, buffer);
			ciphertext = decryptText(buffer);

			fprintf(stdout, "DECSERVER: Outputted Ciphetext: %s\n\n\n\n\n\n\n\n\n", ciphertext);
			


			//Now, repeat on the server side
			packets = 0;
			packets = strlen(ciphertext) / S_BUFSIZE;
			if (strlen(ciphertext) - (packets * S_BUFSIZE) > 0) packets +=1;
			//fprintf(stdout, "ENCSERVER: Num of packets to send over: %d\n", packets);

			memset(temp, '\0', S_BUFSIZE);
			sprintf(temp, "%d\n", packets);
			send(new_socket, temp, S_BUFSIZE, 0);
			//send(sock, temp, S_BUFSIZE, 0);



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
				//memmove(input, input+S_BUFSIZE, BUFSIZE); //Careful with this precision
				send(new_socket, temp, S_BUFSIZE, 0);
			}






			//send(new_socket, hello, strlen(hello), 0);
			//send(new_socket, ciphertext, strlen(ciphertext), 0);
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

//Reads into a small buffer and checks for the correct
//	If it's good, continue; Returns 1
//	If not, then don't; Returns 0
int verifyClient(int new_socket, int valread) {
	//char buffer[BUFSIZE] = {0};
	char newbuffer[S_BUFSIZE] = {0};
	
	//First, receive a message
	valread = recv(new_socket, newbuffer, S_BUFSIZE, 0);
	//printf("ENCSERVER: Message Received: %s\n", buffer);

	//Now, verify the connection if the message is appropriate
	if (strcmp(newbuffer, "DECCLIENT") == 0){
		//printf("ENCSERVER: YOU ARE VERIFIED\n");
		memset(newbuffer, '\0', S_BUFSIZE);
		strcpy(newbuffer, "VERIFIED");
		send(new_socket, newbuffer, S_BUFSIZE, 0);

		return 1;
	} else {
		//printf("ENCSERVER: YOU ARE NOT VERIFIED\n");
		memset(newbuffer, '\0', S_BUFSIZE);
		strcpy(newbuffer, "NOT VERIFIED");
		send(new_socket, newbuffer, S_BUFSIZE, 0);
		//You must also terminate the connection here as well;

		return 0;
		//Increment semaphore now that it's closed
		//shutdown(new_socket, SHUT_RDWR);
		//sem_post(semOpenClients);
		//continue;
	}
		
}


//This actually does the decryption
char* decryptText(char *input) {

	char *token;
	char ciphertext[BUFSIZE] = {0};
	char key[BUFSIZE] = {0};
	char *plaintext = malloc(sizeof(char) * BUFSIZE);
	memset(ciphertext, '\0', BUFSIZE); //Make sure the output buffer for ciphertext is clear
	int keyInt, ptInt, ctInt;

	strcpy(ciphertext, strtok_r(input, "|", &input));
	strcpy(key, strtok_r(input, "|", &input));
	//printf("\nCT: %s\n\tK: %s\n", ciphertext, key);

	//DECRYPTION OPERATION
	//	Same but in reverse, essentially
	//SPECIAL CASES: the ' ' or space character
	//	When read in from outside, it's set to be 
	//	When any char is set to 26 + 'A', it becomes a space (ASCII 32)
	for (int i = 0; i < strlen(ciphertext); i++) {
		//Set spaces to be 'A + 27'
		if (ciphertext[i] == ' ') 
			ciphertext[i] = ('A' + 26);

		if (key[i] == ' ') 
			key[i] = ('A' + 26);

		//printf("CHAR: %c\n", plaintext[i]);

		ctInt = ciphertext[i] - 'A';
		keyInt = key[i] - 'A';

		//This is the only difference; Since we now subtract, just add 26 if the num is negative
		ptInt = (ctInt - keyInt); 
		if (ptInt < 0) ptInt += 27; //add 27 since we have 27 valid chars
		ptInt += 'A';

		//Now convert back to ' ' 
		if (ptInt == (26 + 'A'))
		//if (ptInt == (27 + 'A'))
			ptInt = 32;
		plaintext[i] = ptInt;

	}
	return plaintext;
}

//Outline taken directly from lecture
#define IPADDR "127.0.0.1" // Localhost 
//#define BUFSIZE 1024
#define BUFSIZE 150000
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

#include "enc_client.h"

int main(int argc, char **argv) {
	//String used for all error messaging;
	char idstring [S_BUFSIZE] = "DECCLIENT";

	//Before anything, check if the correct number of parameters are used
	//	NOTE: This uses fprintf to stderr instead of perror
	//	This is because no actual error is found; So, 'errno' prints success
	//	Instead, we have a programmer-defined error state and thus want a custom error message 
	if (argc != 4){
		//perror("ERROR: Incorrect number of parameters\n");
		fprintf(stderr, "%s: ERROR: Incorrect number of parameters\n", idstring);
		//perror("ERROR: Incorrect number of parameters");
		//return -1;
		exit(1);
	}

	//Then, assign vars to the inputted params
	char *plaintext_file = argv[1];
	char *key_file = argv[2];
	int port = atoi(argv[3]);
	char *plaintext = malloc(sizeof(char) * BUFSIZE);
	char *key = malloc(sizeof(char) * BUFSIZE);
	//char *plaintext, *key;
	char *buffer = malloc(sizeof(char) * BUFSIZE);
	memset(buffer, '\0', BUFSIZE);




	if (initialErrorCheck(plaintext, key, argv) == 0) {
		exit(1);
	}


	//With preliminary stuff done, now the connection to the server can be done
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

		
	//Make a socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "%s: ERROR: Socket creation error\n", idstring);
		exit(1);
		//return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); //Maybe make port a cmd line arg or in a shared header?

	//Convert IP address to usable binary
	//if (inet_pton(AF_INET, "localhost", &serv_addr.sin_addr) <= 0) {
	if (inet_pton(AF_INET, IPADDR, &serv_addr.sin_addr) <= 0) {
		fprintf(stderr, "%s: ERROR: Invalid address, address not supported\n", idstring);
		exit(1);
		//return -1;
	}

	//Actually connect to the server
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "%s: ERROR: Connection Failed\n", idstring);
		exit(1);
		//return -1;
	}
	
	

	//setUpSocket(sock, valread, &serv_addr, port);



	//Check if valid server, then perform communication
	if (checkValidServer(sock, valread) == 0) {
		fprintf(stderr, "%s: ERROR: Invalid server.\n", idstring);
		exit(2);
	}





	//Now actually send over plaintext
	memset(buffer, '\0', BUFSIZE);
	sprintf(buffer, "%s|%s", plaintext, key);

	/*
	printf("Num chars sent over the file: %d\n", strlen(buffer));
	printf("Num packets sent over the file if packet size 50: %d\n", strlen(buffer) / 50 + 1);
	*/

	//First, send the number of packets
	int packets = 0;
	char temp[S_BUFSIZE] = {0};
	packets = strlen(buffer) / S_BUFSIZE;
	if (strlen(buffer) - (packets * S_BUFSIZE) > 0) packets +=1;
	//printf("Num of packets to send over: %d\n", packets);
	//send(sock, buffer, BUFSIZE, 0);

	sprintf(temp, "%d\n", packets);
	send(sock, temp, S_BUFSIZE, 0);


	memset(temp, '\0', S_BUFSIZE);
	valread = recv(sock, temp, S_BUFSIZE, 0); //Wait for the confirmation message from the server
	if (strcmp (temp, "go") != 0) {
		fprintf(stderr, "%s: Transmission Failed.\n", idstring);
		exit(1);
	}
	

	for (int i = 0; i < packets; i++) {
		memset(temp, '\0', S_BUFSIZE);
		strncpy(temp, buffer, S_BUFSIZE);	
		//memmove(buffer, buffer+S_BUFSIZE, BUFSIZE); //Careful with this precision
		memmove(buffer, buffer+S_BUFSIZE, BUFSIZE - S_BUFSIZE); //Careful with this precision
		send(sock, temp, S_BUFSIZE, 0);
	}




		
	//First, receive the number of packets
	memset(temp, '\0', S_BUFSIZE);
	valread = recv(sock, temp, S_BUFSIZE, 0);
	char *end;
	packets = (int) strtol(temp, &end, 10);
	//int packets = itoa(temp);
	//printf("RECEIVED THIS AS PACKET COUNT: %d\n", packets);

	//Send confirmation message to send text back
	memset(temp, '\0', S_BUFSIZE);
	strcpy(temp, "go");
	send(sock, temp, S_BUFSIZE, 0);
			



	//Finally, incrementally read in the ciphertext
	//Now, receive the aforementioned number of packets
	memset(buffer, '\0', BUFSIZE);
	for (int i = 0; i < packets; i++) {
		memset(temp, '\0', S_BUFSIZE);
		valread = recv(sock, temp, S_BUFSIZE, 0);
		//printf("ENCSERVER: JUST READ: %s\n", temp);
		strcat(buffer, temp);
	}
	
	char* temp15 = buffer;
	buffer = cleanTransmittedInput(buffer);
	free(temp15);





	//Now read in ciphertext
	/*
	memset(buffer, '\0', BUFSIZE);
	valread = recv(sock, buffer, BUFSIZE, 0); //0 specifies no flags
	//printf("ENCCLIENT: Message received: %s\n", buffer);
	*/

	
	//Clean up input one more time before printing
	if (temp15 = strstr(buffer, "|"))
		*temp15 = '\0';


	//printf("\n\n\n\n\n\n\n\n\n\n\n\n\n%s\n\n\n\n\n\n\n\n\n\n\n", buffer); //Prints out ciphertext
	//printf("%.70000s\n", buffer); //Prints out ciphertext
	printf("%s\n", buffer); //Prints out ciphertext


	//Free memory before exiting
	free(plaintext);
	free(key);
	free(buffer);

	//Exit 0 upon good execution
	exit(0);
	//return 0;
}

char* cleanTransmittedInput(char *buffer) {
	char *newbuf = malloc(sizeof(char) * BUFSIZE);
	memset(newbuf, '\0', BUFSIZE);
	char *token;
	char *saveptr;
	//while (token = strtok_r(buffer, "", &buffer)){ //Removes all ^B chars from the input
	while (token = strtok_r(buffer, "", &buffer)) { //Removes all ^B chars from the input
		strcat(newbuf, token);
	}
	//fprintf(stdout, "Cleaned text:\n%s\n\n\n\n\n\n", newbuf);
	//	fflush(stdout);
	//free(buffer);
	return newbuf;

}




//Returns 1 on success, 0 on fail
int initialErrorCheck(char *plaintext, char *key, char **argv) {
	char *plaintext_file = argv[1];
	char *key_file = argv[2];
	char idstring[S_BUFSIZE] = "DECCLIENT";

	//Now, perform alternate checks
	//If either file is invalid, then exit the program
	//	Error messages printed in function
	if (getStringFromFile(plaintext_file, plaintext) == 0){
		fprintf(stderr, "%s: ERROR: File path for the plaintext is invalid\n", idstring);
		return 0;
	}

	if (getStringFromFile(key_file, key) == 0) {
		fprintf(stderr, "%s: ERROR: File path for the key is invalid\n", idstring);
		return 0;
	}

	//If the key is shorter than the password, also exit
	if (strlen(plaintext) > strlen(key)) {
		fprintf(stderr, "%s: ERROR: Key is too short\n", idstring);
		return 0;
	}

	//Now, check if the characters in the strings are actually valid
	if (checkValidInput(plaintext) == 0) {
		fprintf(stderr, "%s: ERROR: Invalid characters found in the plaintext file\n", idstring);
		return 0;
	}

	if (checkValidInput(key) == 0) {
		fprintf(stderr, "%s: ERROR: Invalid characters found in the key file\n", idstring);
		return 0;
	}


	return 1;

}

int checkValidServer(int sock, int valread) {
	//Request Encryption; 
	//We'll send everything to the server all concatenated at once into the buffer
	//	I think is is good bc it puts the onus locally instead of 50 write calls
	//scanf("%s", msg);
	char temp [S_BUFSIZE];
	memset(temp, '\0', S_BUFSIZE);
	strcpy(temp, "DECCLIENT");
	//strcpy(buffer, "BADENCCLIENT");
	send(sock, temp, S_BUFSIZE, 0);
	//printf("ENCCLIENT: ID STRING sent\n");

	//Before reading back from server, clear buffer
	memset(temp, '\0', S_BUFSIZE);
	valread = recv(sock, temp, S_BUFSIZE, 0); //0 specifies no flags

	//printf("ENCCLIENT: Checking if we are valid to continue: %s\n", buffer);

	if (strcmp(temp, "NOT VERIFIED") == 0) 
		return 0;
		//fprintf(stderr, "ENCCLIENT: ERROR: Invalid server.\n");
		//exit(2);
	

	return 1;
}


//Returns 0 on fail, 1 on success
//Takes from the file name and copies it into the outputStr
int getStringFromFile(char *filePath, char *outputStr) {
//int getStringFromFile(char *fileName, char outputStr) {
	//Now, actually read in the values from the file
    	FILE *istr; 
    	size_t len = 0;
    	ssize_t nread;
	char *currLine = NULL;

	//Read in a line, copy it into the given string ptr
	//	Remember to remove the final newline as well by making it a '\0'
	if (istr = fopen(filePath, "r")) {
		nread = getline(&currLine, &len, istr); 
		strcpy(outputStr, currLine);
		outputStr[strlen(outputStr) - 1] = '\0'; //Removes newline; MAYBE BUGGED WITH DIF INPUT?
		
		free(currLine);
		fclose(istr);
		return 1;
	} else {
		//perror("ERROR: File path '%s' is invalid\n", filePath);
		return 0;
	}


}

//Returns 1 if input is valid, 0 if it isn't
int checkValidInput(char *input) {
	//Run through the string; If anything is NOT a capital letter or ' ', return 0
	for (int i = 0; i < strlen(input); i++) 
		if ( (input[i] < 'A' || input[i] > 'Z') && input[i] != ' ')
			return 0;

	return 1;
}



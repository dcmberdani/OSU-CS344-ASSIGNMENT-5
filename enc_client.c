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

#include "enc_client.h"

void testprint() {
	printf("Nothing has crashed and burned yet.\n");
}

int main(int argc, char **argv) {
	//Before anything, check if the correct number of parameters are used
	if (argc != 4){
		printf("ERROR: Incorrect number of parameters\n");
		return -1;
	}

	//Then, assign vars to the inputted params
	char *plaintext_file = argv[1];
	char *key_file = argv[2];
	int PORT = atoi(argv[3]);
	char *plaintext = malloc(sizeof(char) * BUFSIZE);
	char *key = malloc(sizeof(char) * BUFSIZE);

	printf("ENCCLIENT; PLAINTEXT FILE: %s\n\tKEY FILE: %s\n\tPORT: %d\n", plaintext_file, key_file, PORT);

	//Now, perform alternate checks
	//If either file is invalid, then exit the program
	//	Error messages printed in function
	if (getStringFromFile(plaintext_file, plaintext) == 0) 
		return -1;

	if (getStringFromFile(key_file, key) == 0) 
		return -1;
	

	//If the key is shorter than the password, also exit
	if (strlen(plaintext) > strlen(key)) {
		printf("ERROR: key in '%s' is too short \n", key_file);
		return -1;
	}


	printf("ENCCLIENT; PLAINTEXT: %s\n\tKEY: %s\n", plaintext, key);

	//With preliminary stuff done, now the connection to the server can be done


	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	//char *hello = "ENCCLIENT: Hello[Client]";
	char msg[BUFSIZE] = {0};
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
	scanf("%s", msg);
	//send(sock, hello, strlen(hello), 0);
	send(sock, msg, strlen(msg), 0);
	printf("ENCCLIENT: Message sent\n");
	//valread = recv(sock, buffer, BUFSIZE);
	valread = recv(sock, buffer, BUFSIZE, 0); //0 specifies no flags
	printf("ENCCLIENT: Message received: %s\n", buffer);

	//testprint();
	
	free(plaintext);
	free(key);

	return 0;
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
		return 1;
	} else {
		printf("ERROR: File path '%s' is invalid\n", filePath);
		return 0;
	}


}


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

int main(int argc, char **argv) {
	//Before anything, check if the correct number of parameters are used
	//	NOTE: This uses fprintf to stderr instead of perror
	//	This is because no actual error is found; So, 'errno' prints success
	//	Instead, we have a programmer-defined error state and thus want a custom error message 
	if (argc != 4){
		//perror("ERROR: Incorrect number of parameters\n");
		fprintf(stderr, "ENCCLIENT: ERROR: Incorrect number of parameters\n");
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
		perror("ENCCLIENT: ERROR: Socket creation error");
		exit(1);
		//return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); //Maybe make port a cmd line arg or in a shared header?

	//Convert IP address to usable binary
	//if (inet_pton(AF_INET, "localhost", &serv_addr.sin_addr) <= 0) {
	if (inet_pton(AF_INET, IPADDR, &serv_addr.sin_addr) <= 0) {
		perror("ENCCLIENT: ERROR: Invalid address, address not supported");
		exit(1);
		//return -1;
	}

	//Actually connect to the server
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("ENCCLIENT: ERROR: Connection Failed");
		exit(1);
		//return -1;
	}
	
	

	//setUpSocket(sock, valread, &serv_addr, port);



	//Check if valid server, then perform communication
	if (checkValidServer(sock, valread, buffer) == 0) {
		fprintf(stderr, "ENCCLIENT: ERROR: Invalid server.\n");
		exit(2);
	}





	//Now actually send over plaintext
	memset(buffer, '\0', BUFSIZE);
	sprintf(buffer, "%s|%s", plaintext, key);
	send(sock, buffer, BUFSIZE, 0);


	//Now read in ciphertext
	memset(buffer, '\0', BUFSIZE);
	valread = recv(sock, buffer, BUFSIZE, 0); //0 specifies no flags
	//printf("ENCCLIENT: Message received: %s\n", buffer);
	printf("%s\n", buffer); //Prints out ciphertext


	//Free memory before exiting
	free(plaintext);
	free(key);
	free(buffer);

	//Exit 0 upon good execution
	exit(0);
	//return 0;
}


//Returns 1 on success, 0 on fail
int initialErrorCheck(char *plaintext, char *key, char **argv) {
	char *plaintext_file = argv[1];
	char *key_file = argv[2];

	//Now, perform alternate checks
	//If either file is invalid, then exit the program
	//	Error messages printed in function
	if (getStringFromFile(plaintext_file, plaintext) == 0){
		perror("ENCCLIENT: ERROR: File path for the plaintext is invalid");
		return 0;
	}

	if (getStringFromFile(key_file, key) == 0) {
		perror("ENCCLIENT: ERROR: File path for the key is invalid");
		return 0;
	}

	//If the key is shorter than the password, also exit
	if (strlen(plaintext) > strlen(key)) {
		fprintf(stderr, "ENCCLIENT: ERROR: Key is too short\n");
		return 0;
	}

	//Now, check if the characters in the strings are actually valid
	if (checkValidInput(plaintext) == 0) {
		fprintf(stderr, "ENCCLIENT: ERROR: Invalid characters found in the plaintext file\n");
		return 0;
	}

	if (checkValidInput(key) == 0) {
		fprintf(stderr, "ENCCLIENT: ERROR: Invalid characters found in the key file\n");
		return 0;
	}


	return 1;

}

int checkValidServer(int sock, int valread, char *buffer) {
	//Request Encryption; 
	//We'll send everything to the server all concatenated at once into the buffer
	//	I think is is good bc it puts the onus locally instead of 50 write calls
	//scanf("%s", msg);
	memset(buffer, '\0', BUFSIZE);
	strcpy(buffer, "ENCCLIENT");
	//strcpy(buffer, "BADENCCLIENT");
	send(sock, buffer, BUFSIZE, 0);
	//printf("ENCCLIENT: ID STRING sent\n");

	//Before reading back from server, clear buffer
	memset(buffer, '\0', BUFSIZE);
	valread = recv(sock, buffer, BUFSIZE, 0); //0 specifies no flags

	//printf("ENCCLIENT: Checking if we are valid to continue: %s\n", buffer);

	if (strcmp(buffer, "NOT VERIFIED") == 0) 
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


//This actually does the encryption
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

#include <stdio.h>
#include <stdlib.h> //rand(), atoi/itoa()
#include <time.h> //time()

#include "keygen.h"

int main(int argc, char **argv) {
	//First, grab the key length from the arguments
	if (argc != 2){
		printf("ERROR: Incorrect number of parameters\n");
		return -1;
	}

	int keylen = atoi(argv[1]);

	//Now, seed the rng and start to generate the key
	srand(time(NULL));

	generateKey(keylen);

	printf("\n");

	return 0;
}

void generateKey(int keylen) {
	//Generate a random number between 65 and 90 inclusive (ASCII A-Z)
	//	Then, convert that int into a char, and write it to stdout	
	int randint;
	for (int i = 0; i < keylen; i++) {
		randint = rand() % 26 + 65;
		//printf("%d", randint);
		printf("%c", randint); //Print out the int with char formatting
	}
}



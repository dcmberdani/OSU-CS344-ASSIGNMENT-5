#ifndef ENC_SERVER
#define ENC_SERVER


//Encrypts text
char* encryptText(char *plaintext);

//Reads into a small buffer and checks for the correct
//	If it's good, continue; Returns 1
//	If not, then don't; Returns 0
int verifyClient(int new_socket, int valread);

//Removes the '^B's everywhere
char* cleanTransmittedInput(char *buffer);

#endif

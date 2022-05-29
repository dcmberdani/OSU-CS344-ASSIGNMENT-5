#ifndef DEC_SERVER
#define DEC_SERVER


//Encrypts text
char* decryptText(char *input);

//Reads into a small buffer and checks for the correct
//	If it's good, continue; Returns 1
//	If not, then don't; Returns 0
int verifyClient(int new_socket, int valread);

//Removes the '^B's everywhere
char* cleanTransmittedInput(char *buffer);

#endif

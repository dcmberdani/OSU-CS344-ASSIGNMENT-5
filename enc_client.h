#ifndef ENC_CLIENT
#define ENC_CLIENT


//Grabs a string from a file; Returns an int depending on success
int getStringFromFile(char *filePath, char *outputStr);
//int getStringFromFile(char *fileName, char outputStr);


//Checks for the initial set of errors
int initialErrorCheck(char *plaintext, char *key, char **argv);

//Checks if the inputted plaintext/key have any invalid chars or not
int checkValidInput(char *input);

//Checks if the server is the correct 
int checkValidServer(int sock, int valread);


char* cleanTransmittedInput(char *buffer);

#endif

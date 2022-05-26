#ifndef ENC_CLIENT
#define ENC_CLIENT


//Grabs a string from a file; Returns an int depending on success
int getStringFromFile(char *filePath, char *outputStr);
//int getStringFromFile(char *fileName, char outputStr);


//Checks if the inputted plaintext/key have any invalid chars or not
int checkValidInput(char *input);

//JUST FOR TESTING
char* decryptText(char *input);

#endif

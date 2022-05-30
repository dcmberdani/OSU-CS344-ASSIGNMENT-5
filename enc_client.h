#ifndef ENC_CLIENT
#define ENC_CLIENT


//Grabs a string from a file; Returns an int depending on success
/*
 * Name: getStringFromFile
 * Description: Grabs a string from a given filepath, places it in the output string
 * 	An integer is returned to determine if the filepath exists or not
 * Preconditions: A file path
 * Postconditions: Either the string from the filepath, or an error indicating failure
 */
int getStringFromFile(char *filePath, char *outputStr);


/*
 * Name: initialErrorCheck
 * Description: Runs through a preliminary set of error checks for the inputted plaintext/key
 * 	These include checking for invalid characters, length etc. 
 * Preconditions: Plaintext/key files being provided in the command line arguments
 * Postconditions: Ensures that only valid input is sent through the program
 */
int initialErrorCheck(char *plaintext, char *key, char **argv);


/*
 * Name: checkValidInput
 * Description: Runs through a given string
 * 	Ensures that only capital letters and space characters are in the string
 * 	Returns an integer representing the found outcome
 * Preconditions: A string
 * Postconditions: Whether or not that string is valid
 */
int checkValidInput(char *input);


/*
 * Name: checkValidServer
 * Description: Performs an exchange with a given server/socket
 * 	Does so by initially sending a message with ID of the client (ENCCLIENT here)
 * 	THe server has a partner function that receives the ID, and validates it
 * 	This function waits for a response
 * 		If validated, the program continues
 * 		If not, the program ends
 * Preconditions: A server to communicate to being established
 * Postconditions: Ensures only the correct server is communicated with
 */
int checkValidServer(int sock, int valread);


/*
 * Name: cleanTransmittedInput
 * Description: When sending messages with my current protocol, each packet has '^B' at the start
 * 	These characters mess up the encryption/decryption and string validation
 * 	So, this function simply removes all the '^B' characters
 * Preconditions: A string containing '^B' characters
 * Postconditions: A string without the chars
 */
char* cleanTransmittedInput(char *buffer);

#endif

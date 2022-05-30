#ifndef ENC_SERVER
#define ENC_SERVER


//Encrypts text
/*
 * Name: encryptText
 * Description: Encrypts the plaintext using the substitution protocl provided on the assignment page
 * 	The only difference is that values are using a 27-length alphabet, where the final char is ' '
 * 	So, ' ' chars are converted to 27, encrypted/decrypted; Remaining 27s are then converted to ' 's
 * Preconditions: A string
 * Postconditions: An encoded string
 */
char* encryptText(char *plaintext);

/*
 * Name: verifyClient
 * Description: Companion function to one used by the clients
 * 	Receives a message from the client, then checks to see if it's the correct one
 * 	If so, send back a validation message
 * 	If not, then send back a non-validation message and close the socket
 * Preconditions: A client sending an ID message
 * Postconditions: A message sent back to the client regarding the state of the connection
 */
int verifyClient(int new_socket, int valread);

/*
 * Name: cleanTransmittedInput
 * Description: Identical to the version in the client header
 * Preconditions: a string
 * Postconditions: A string with all '^B' chars removed
 */
char* cleanTransmittedInput(char *buffer);

#endif

/*
 * CITATION NOTE: I used code from client.c and server.c, as well as the lectures, for this program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define bufferSize	75000 // Used to set the buffer size

// Function to send a message to the server
void sendMessage(int sockFD, FILE* msgFD, int msgSize) {
	int i, charsWritten;
	char msg[bufferSize]; // Holds the message to send to the server
	char c;

	memset(msg, '\0', sizeof(msg)); // Ensures the buffer is clear

	// Loops to read each character of the message
	for(i = 0; i < msgSize; i++) {
		// For the last character of the message
		if(i == msgSize-1) {
			msg[i] = 'z'; // 'z' will be used on the server side to determine the end of the message
		}
		// For all other characters of the message
		else {
			c = getc(msgFD); // Puts the character in variable "c"

			// If the character is valid
			if(isspace(c) || isupper(c)) {
				msg[i] = c; // Adds the character to the "msg" array
			}
			// If the character is not valid
			else {
				fprintf(stderr, "Error: Invalid character\n"); // Prints error message
				exit(1); // Exits the program
			}
		}
	}

	fclose(msgFD); // Closes the file

	msg[i] = '\0'; // Sets final character of message to null terminator

	charsWritten = send(sockFD, msg, strlen(msg), 0); // Sends the message

	// Prints error message if the message was not sent
	if(charsWritten < 0) {
		fprintf(stderr, "CLIENT: ERROR writing to socket\n");
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsRead, cipherSize, keySize;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[bufferSize];
	char authMsg[] = "Connected to otp_dec"; // Message to send to the server for authentication purposes
	FILE *cipherFD, *keyFD;

	// Checks for correct number of arguments
	if(argc != 4) {
		// Prints error message and exits, if incorrect number of arguments
		fprintf(stderr, "Error: Incorrect number of arguments.\n");
		exit(0);
	}

	// Sets up the server address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[3]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverHostInfo = gethostbyname("localhost");

	// Checks that the host is valid
	if(serverHostInfo == NULL) {
		// Prints error message and exits if not
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}

	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copies in the address

	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Creates the socket

	// If the socket was not successfully opened
	if(socketFD < 0) {
		fprintf(stderr, "CLIENT: ERROR opening socket\n");
		exit(0);
	}

	// Attemps to connect the socket
	if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		// Prints error message and exits if connection was unsuccessful
		fprintf(stderr, "CLIENT: ERROR connecting\n");
		exit(0);
	}

	// Messages to perform a handshake to ensure correct programs are running
	write(socketFD, authMsg, sizeof(authMsg)); // Sends the authentication method to the server
	read(socketFD, buffer, sizeof(buffer)); // Receives the server's response

	// If the response was not run from otp_dec_d.c
	if(strcmp(buffer, "Connected to otp_dec_d") != 0) {
		fprintf(stderr, "Error: could not contact otp_enc_d on port %d.\n", portNumber); // Prints error message
		exit(2); // Terminates the process and exits
	}

	// Attempts to open the file containing the ciphertext
	cipherFD = fopen(argv[1], "r");

	// Prints error message and exits if unsuccessful
	if(cipherFD < 0) {
		fprintf(stderr, "ERROR opening cipher file.\n");
		exit(1);
	}

	// Gets the size of the ciphertext file and resets the file position to the beginning
	fseek(cipherFD, 0L, SEEK_END);
	cipherSize = ftell(cipherFD);
	fseek(cipherFD, 0L, SEEK_SET);

	// Attempts to open the file containing the key
	keyFD = fopen(argv[2], "r");

	// Prints error message and exits if unsuccessful
	if(keyFD < 0) {
		fprintf(stderr, "ERROR opening key file.\n");
		exit(1);
	}

	// Gets the size of the key file and resets the file position to the beginning
	fseek(keyFD, 0L, SEEK_END);
	keySize = ftell(keyFD);
	fseek(keyFD, 0L, SEEK_SET);

	// Checks that the key size and ciphertext size are compatible
	if(keySize < cipherSize) {
		// Prints error message and exits if not
		fprintf(stderr, "ERROR: key is too short.\n");
		exit(1);
	}

	sendMessage(socketFD, cipherFD, cipherSize); // To send the ciphertext to the server
	sendMessage(socketFD, keyFD, keySize); // To send the key to the server

	memset(buffer, '\0', sizeof(buffer)); // Clears the buffer

	charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); // Gets the decrypted message back from the server

	// Prints error message and exits if reading the message failed
	if(charsRead < 0) {
		fprintf(stderr, "CLIENT: ERROR reading from socket.\n");
		exit(0);
	}

	printf("%s\n", buffer); // Prints the message

	close(socketFD); // Closes the socket

	return 0;
}

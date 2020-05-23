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

#define bufferSize	75000

// An array of the available characters
const char chars[27] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
				'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
				'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ' };

// Determines the index of a character in the above array
int getIntVal(char c) {
	// Loops for each element of the array
	for(int i = 0; i < 27; i++) {
		if(c == chars[i]) { // Checks if the current element is a match
			return i; // Returns the index if it is
		}
	}

	return -1; // In case none of the characters matches
}

// Function to decrypt the ciphertext
void decryptMessage(int socketFD, char* cipher, char* key) {
	int i, cipherIdx, keyIdx, msgIdx;

	// Loops for each character in the ciphertext
	for(i = 0; i < strlen(cipher); i++) {
		cipherIdx = getIntVal(cipher[i]); // Gets the index for the cihpertext
		keyIdx = getIntVal(key[i]); // Gets the index for the key
		msgIdx = cipherIdx - keyIdx; // Determines the index to decrypt the ciphertext

		// If the index is negative, adds 27 to make it positive
		if(msgIdx < 0) {
			msgIdx += 27;
		}

		cipher[i] = chars[msgIdx % 27]; // Adds the character at the index to the decrypted message
	}

	cipher[i] = '\0'; // Sets the final character to a null terminator
}

// Function to receive a message from the client
void getMessage(int socketFD, char* buffer) {
	char readBuffer[10];

	// Continues reading characters until it encounters "z", indicating the end of the message
	while(strstr(buffer, "z") == NULL) {
		memset(readBuffer, '\0', sizeof(readBuffer)); // Clears the buffer
		int r = recv(socketFD, readBuffer, sizeof(readBuffer)-1, 0); // Gets the next part of the message
		strcat(buffer, readBuffer); // Copies into the buffer for the full message
	}

	int terminalLocation = strstr(buffer, "z") - buffer; // Determines the index where "z" appears
	buffer[terminalLocation] = '\0'; // Sets that index to a null terminator
}

int main(int argc, char *argv[]) {
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char cipherBuffer[bufferSize];
	char keyBuffer[bufferSize];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnpid; 

	// Checks that number of arguments is correct
	if(argc != 2) {
		// Prints error message and exits if not
		fprintf(stderr, "Error: Incorrect number of arguments.\n");
		exit(0);
	}

	// Sets up the struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[1]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Creates the socket

	// Prints error message and exits if socket was not successfully opened
	if(listenSocketFD < 0) {
		fprintf(stderr, "ERROR opening socket.\n");
		exit(1);
	}

	// Attempts to bind the socket
	if(bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		// Prints error message and exits if unsuccessful	
		fprintf(stderr, "ERROR on binding.\n");
		exit(1);
	}

	listen(listenSocketFD, 5); // Listens for connections; can receive up to 5 at once

	// Loops until program exits
	while(1) {
		spawnpid = -5; // Sets to junk value

		// Accepts a connection
		sizeOfClientInfo = sizeof(clientAddress);
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);

		// Prints error message and exits if connection was unsuccessful
		if(establishedConnectionFD < 0) {
			fprintf(stderr, "ERROR on accept.\n");
			exit(1);
		}

		spawnpid = fork(); // Forks off a new process

		switch(spawnpid) {
			// If there was an error forking
			case -1:
				// Prints error message and exits
				perror("ERROR on fork");
				exit(1);
				break;
			// If fork was successful
			case 0:
				memset(cipherBuffer, '\0', sizeof(cipherBuffer)); // Clears the buffer
				charsRead = recv(establishedConnectionFD, cipherBuffer, sizeof(cipherBuffer)-1, 0); // Receives a message for the handshake

				// If not connected using the correct program
				if(strcmp(cipherBuffer, "Connected to otp_dec") != 0) {
					// Sends error message and exits
					char msg[] = "Invalid connection. Closing connection.";
					write(establishedConnectionFD, msg, sizeof(msg));
					exit(2);
				}
				// If connected using the correct program
				else {
					char msg[] = "Connected to otp_dec_d";
					write(establishedConnectionFD, msg, sizeof(msg)); // Sends back a message to complete the handshake
				}

				// Clears the buffers
				memset(cipherBuffer, '\0', sizeof(cipherBuffer)); 
				memset(keyBuffer, '\0', sizeof(keyBuffer));

				getMessage(establishedConnectionFD, cipherBuffer); // Gets the ciphertext
				getMessage(establishedConnectionFD, keyBuffer); // Gets the key

				decryptMessage(establishedConnectionFD, cipherBuffer, keyBuffer); // Calls function to decrypt the message

				write(establishedConnectionFD, cipherBuffer, sizeof(cipherBuffer)); // Sends the decrypted message back to the client
		}

		close(establishedConnectionFD); // Closes the connection with the client
	}

	close(listenSocketFD); // Closes the socket

	return 0;
}

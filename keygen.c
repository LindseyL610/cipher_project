#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateKey(int keyLength) {
	// Array of all valid characters
	char keyChars[27] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
				'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
				'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ' };

	// Loops for the chosen length
	for(int i = 0; i < keyLength; i++) {
		int idx = rand() % 27; // Selects random index 
		fprintf(stdout, "%c", keyChars[idx]); // Prints the character at that index
	}

	fprintf(stdout, "\n"); // Prints newline
}

int main(int argc, char *argv[]) {
	// If number of arguments is invalid
	if(argc != 2) {
		// Prints error message and exits
		fprintf(stderr, "Incorrect number of arguments.\n");
		exit(0);
	}
	// If number of arguments is valid
	else {
		srand(time(NULL));
		generateKey(atoi(argv[1])); // Generates the key
	}

	return 0;
}

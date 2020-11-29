#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[])
{
	// Error check if no input was given
	if (argc < 2)
	{
		fprintf(stderr, "Error: Please enter desired amount for keylength\n");
		return 0;
	}
	// Convert input to int
	int keylength = atoi(argv[1]);
	// Create key buffer
	char key[keylength+3];
	memset(key, 0, sizeof(key));
	// Create alphabet reference 
	char alphabet[27] =  " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int r;
	// Genereate key
	for (int i = 0; i < keylength; i++) 
	{
		r = rand() % 27;
		key[i] = alphabet[r];
	}
	key[keylength] = '\n';
	fprintf(stdout, "%s", key);
}
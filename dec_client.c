#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Checks if string contains any bad characters. If so, print error
void badCharacters(char* string)
{
    int stringLength = strlen(string);
    for (int i = 0; i < stringLength; i++) {
        char ch = string[i];
        if (isalnum(ch) == 0 && ch != ' ') {
            printf("Character is : %c \n", ch);
            perror("ERROR: A FILE CONTAINS INVALID CHARACTER");
            exit(1);
        }
    }
}


// Grab text from file and adds it to buffer
void textFile(char* buffer, char* fileName)
{
    FILE* textFile = fopen(fileName, "r");
    long  numbytes;

    // Find out how many bytes is the text file
    fseek(textFile, 0L, SEEK_END);
    numbytes = ftell(textFile);
    // Reset the file position indicator 
    fseek(textFile, 0L, SEEK_SET);
    // Add text to buffer
    fread(buffer, sizeof(char), numbytes, textFile);
    fclose(textFile);
}

void keyFile(char* buffer, char* fileName)
{
    FILE* textFile = fopen(fileName, "r");
    long  numbytes;

    // Find out how many bytes is the text file
    fseek(textFile, 0L, SEEK_END);
    numbytes = ftell(textFile);
    // Reset the file position indicator 
    fseek(textFile, 0L, SEEK_SET);
    // Add text to buffer
    fread(buffer, sizeof(char), numbytes, textFile);
    fclose(textFile);
}

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber,
    char* hostname) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char* argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[70000];
    char key[70000];
    // Check usage & args
    if (argc < 4) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }
    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }
    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }
    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Grab text from file and put into buffer
    textFile(buffer, argv[1]);
    // Grab text from file and put into key
    keyFile(key, argv[2]);
    // Remove the trailing \n that gets added
    buffer[strcspn(buffer, "\n")] = '\0';
    key[strcspn(key, "\n")] = '\0';
    // Error check buffer and key; key not big enough or invalid characters
    if (strlen(key) < strlen(buffer)) {
        perror("ERROR: KEY IS TOO SHORT");
        exit(1);
    }
    badCharacters(key);
    badCharacters(buffer);

    // Pointer for buffer
    const char* p = buffer;
    int length = strlen(buffer);
    // Send message to server
    // Write to the server
    while (length > 0) {
        charsWritten = send(socketFD, p, length, 0);
        if (charsWritten < 0) {
            error("CLIENT: ERROR writing to socket");
        }
        p += charsWritten;
        length -= charsWritten;
    }
    // Send to Server, let it know that the message is finished sending
    charsWritten = send(socketFD, "@@", 2, 0);
    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }
    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Close the socket
    close(socketFD);
    return 0;
}
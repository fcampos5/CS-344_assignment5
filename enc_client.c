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
    // CREATE SOCKET -------------------------------------------------------------------
    int socketFD, portNumber, charsWritten, charsRead, sendID, recvID;
    struct sockaddr_in serverAddress;
    char buffer[140000];
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
    // ID CHECK -------------------------------------------------------------------------
    // Send identification to server
    char bufferID[3];
    int bufferLength = sizeof(bufferID);
    memset(bufferID, '\0', sizeof(bufferID));
    // Send identification
    while(bufferLength > 0){
        sendID = send(socketFD, "enc", 3, 0);
        if (sendID < 0) {
            error("CLIENT: ERROR writing to socket");
        }
        bufferLength -= sendID;
    }
    // Recv identification confirmation
    sendID = 0;
    bufferLength = sizeof(bufferID);
    while (bufferLength > 0) {
        sendID = recv(socketFD, bufferID, 3, 0);
        if (sendID < 0) {
            error("CLIENT: ERROR writing to socket");
        }
        bufferLength -= sendID;
    }
    // Error check confirmation
    if (strcmp(bufferID, "enc") != 0) 
    {
        perror("CLIENT: INVALID CONNECTION TO SERVER");
        exit(2);
    }
    // FILE CHECK -------------------------------------------------------------------------
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
    if (strlen(key) < strlen(buffer)){
        perror("ERROR: KEY IS TOO SHORT");
        exit(1);
    }
    badCharacters(key);
    badCharacters(buffer);
    // Combine key with buffer
    strcat(buffer, "/");
    strcat(buffer, key);
    // SEND MESSAGE -------------------------------------------------------------------------
    // Send message to server
    const char* p = buffer;
    int length = strlen(buffer);
    while(length > 0){
        charsWritten = send(socketFD, p, length, 0);
        if (charsWritten < 0) {
            error("CLIENT: ERROR writing to socket");}
        p += charsWritten;
        length -= charsWritten;
    }
    // Send to Server, let it know that the message is finished sending
    charsWritten = send(socketFD, "@@", 2, 0);

    // RECV MESSAGE -------------------------------------------------------------------------
    memset(buffer, '\0', sizeof(buffer));
    const char* pm = buffer;
    // Read the encrypted message from the socket
    while (1) {
        charsRead = recv(socketFD, pm, 70000, 0);
        if (strstr(buffer, "@@") != NULL)
        {
            buffer[strcspn(buffer, "@@")] = '\0';
            break;
        }
        if (charsRead < 0) {
            error("ERROR reading from socket");
        }
        pm += charsRead;
    }
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    // Close the socket
    close(socketFD);
    return 0;
}
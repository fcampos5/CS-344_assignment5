#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Returns int value of a char
int charToNumber(char* character) 
{
    if (character == ' ')
        return 0;
    if (character == 'A')
        return 1;
    if (character == 'B')
        return 2;
    if (character == 'C')
        return 3;
    if (character == 'D')
        return 4;
    if (character == 'E')
        return 5;
    if (character == 'F')
        return 6;
    if (character == 'G')
        return 7;
    if (character == 'H')
        return 8;
    if (character == 'I')
        return 9;
    if (character == 'J')
        return 10;
    if (character == 'K')
        return 11;
    if (character == 'L')
        return 12;
    if (character == 'M')
        return 13;
    if (character == 'N')
        return 14;
    if (character == 'O')
        return 15;
    if (character == 'P')
        return 16;
    if (character == 'Q')
        return 17;
    if (character == 'R')
        return 18;
    if (character == 'S')
        return 19;
    if (character == 'T')
        return 20;
    if (character == 'U')
        return 21;
    if (character == 'V')
        return 22;
    if (character == 'W')
        return 23;
    if (character == 'X')
        return 24;
    if (character == 'Y')
        return 25;
    if (character == 'Z')
        return 26;
}

// Return char value of an int
char numberToChar(int number)
{
    char alphabet[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return (alphabet[number]);
}

void encrypt(char* encryptMessage, char* message, char* key) 
{
    // messageNum will hold int values of ((message + key) mod 27)
    int size = strlen(message);
    int messageNum[size];
    // Add encrypted int values to messageNum
    for (int i = 0; i < size; i++) {
        messageNum[i] = (charToNumber(message[i]) + charToNumber(key[i])) % 27;
    }
    // Convert int values to char and add to encrypted message
    for (int j = 0; j < size; j++) 
    {
        encryptMessage[j] = numberToChar(messageNum[j]);
    }
}

// Extracts the message and key from the buffer
void splitMessage(char* buffer, char* message, char* key) 
{
    int stop = strlen(buffer);
    int i = 0;
    int j = 0;
    // Extract message
    while (buffer[i] != '/'){
        message[i] = buffer[i];
        i++;
    }
    i++;
    // Extract key
    while (i < stop){
        key[j] = buffer[i];
        i++;
        j++;
    }
}

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
    // CREATE SOCKET -------------------------------------------------------------------------
    int connectionSocket, charsRead, confirmID;
    char buffer[140000];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }
    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }
    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));
    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }
    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);
    // ACCEPT CONNECTIONS -------------------------------------------------------------------------
    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }
        // CHECK CLIENT ID -------------------------------------------------------------------------
        // Confirm if client is allowed on server by checking ID sent by client
        char clientID[3];
        int clientIDLength = sizeof(clientID);
        // Recv identification from client
        while (clientIDLength > 0) {
            confirmID = recv(connectionSocket, clientID, 3, 0);
            if (confirmID < 0) {
                error("CLIENT: ERROR writing to socket");
            }
            clientIDLength -= confirmID;
        }
        // Send confirmation to client
        confirmID = 0;
        clientIDLength = sizeof(clientID);
        while (clientIDLength > 0) {
            confirmID = send(connectionSocket, "enc", 3, 0);
            if (confirmID < 0) {
                error("CLIENT: ERROR writing to socket");
            }
            clientIDLength -= confirmID;
        }
        // Stop connection if incorrect client 
        if (strcmp(clientID, "enc") != 0) {
            close(connectionSocket);
        }
        else {
            // RECV MESSAGE -------------------------------------------------------------------------
            // Get the message from the client 
            memset(buffer, '\0', sizeof(buffer));
            const char* p = buffer;
            // Read the client's message from the socket
            while (1){
                charsRead = recv(connectionSocket, p, 140000, 0);
                if (strstr(buffer, "@@") != NULL)
                {
                    buffer[strcspn(buffer, "@@")] = '\0';
                    break;
                }
                if (charsRead < 0) {
                    error("ERROR reading from socket");
                }
                p += charsRead;
            }
           // printf("SERVER: I received this from the client: \"%s\"\n", buffer);
            // SPLIT MESSAGE -------------------------------------------------------------------------
            char message[70000];
            char key[70000];
            char* token;
            // Set memory for message and key
            memset(message, '\0', sizeof(message));
            memset(key, '\0', sizeof(key));
            // Spilt buffer into message and key
            splitMessage(buffer, message, key);
            // ENCRYPT MESSAGE -------------------------------------------------------------------------
            char encryptMessage[70000];
            encrypt(encryptMessage, message, key);
            // SEND MESSAGE -------------------------------------------------------------------------
            // Send a encrpted message back to the client
            const char* pm = encryptMessage;
            int length = strlen(encryptMessage);
            charsRead = 0;
            while (length > 0) {
                charsRead = send(connectionSocket, pm, length, 0);
                if (charsRead < 0) {
                    error("CLIENT: ERROR writing to socket");
                }
                pm += charsRead;
                length -= charsRead;
            }
            // Send to Server, let it know that the message is finished sending
            charsRead = send(connectionSocket, "@@", 2, 0);
            // Close the connection socket for this client
            close(connectionSocket);
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//void encrypt(char* buffer, char* message) 
//{
//    int array[sizeof(buffer)];
//
//}



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
    char buffer[70000];
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
            printf("WRONG ID \n");
            close(connectionSocket);
        }
        else {
            // RECV MESSAGE -------------------------------------------------------------------------
            // Get the message from the client and display it
            memset(buffer, '\0', sizeof(buffer));
            const char* p = buffer;
            // Read the client's message from the socket
            while (1){
                charsRead = recv(connectionSocket, p, 70000, 0);
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
            printf("SERVER: I received this from the client: \"%s\"\n", buffer);
            // LET CLIENT KNOW FINISHED REC ------------------------------------------------------

            // RECV KEY -------------------------------------------------------------------------

            // ENCRYPT MESSAGE -------------------------------------------------------------------------
            //char message[70000];
            //memset(message, '\0', sizeof(message));
            //encrypt(buffer, message);


            // SEND MESSAGE -------------------------------------------------------------------------
            // Send a Success message back to the client
            //charsRead = send(connectionSocket,
            //    key, 39, 0);
            //if (charsRead < 0) {
            //    error("ERROR writing to socket");
            //}
            // Close the connection socket for this client
            close(connectionSocket);
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}

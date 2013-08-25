/*
 * Server TCP/IP Multiple-Client Forking.
 * Author : Vram Voskanyan
 * Review : Koriun Aslanyan.
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
// socket
#include <sys/socket.h>
#include <sys/types.h>
// IP Address conversion
#include <arpa/inet.h>
// INET constants
#include <netinet/in.h>
// vector 
#include <vector>
// threads
#include <pthread.h>

// port number to use
#define PORT 53008
// size of the buffer for incoming data
#define BUF_SIZE 1024
// maximum size of the pending connections queue
#define QUEUE_SIZE 10

// descriptor for the socket we'll use as the server
int serverSocket;
// descriptor for an accepted connection to a client
int clientSocket;
// socket internet address data for the server
struct sockaddr_in serverData;
// socket internet address data for a client
struct sockaddr_in clientData;
unsigned clientDataLength;
// buffer for incoming/outgoing data
char buffer[BUF_SIZE];
bool isServing;
bool isConnectedToClient;
int bytesReceived;
// process ID
int pid;
// for bind the socket to the address
int bindResult;
// clients sockets
std::vector<int> clientSockets;

// Sender function for sender thread
void* dataSender(void* ptr) {
	isConnectedToClient = true;
	while (isConnectedToClient) {
		// get client ID
		int id;
		printf("Enter client id: ");
		        fgets(buffer, BUF_SIZE, stdin);
		        sscanf(buffer, "%d", &id);
		        // send some data
		        printf("Send> ");
		        fgets(buffer, BUF_SIZE, stdin);
				send(clientSockets[id], buffer, strlen(buffer), 0);
	}
}

void* dataReceiver(void *ptr) {
	isConnectedToClient = true;
	while (isConnectedToClient) {
		// receive any data from the client
		bytesReceived = recv(clientSocket, buffer, BUF_SIZE, 0);

		// terminate the bytes as a string and print the result
		buffer[bytesReceived] = '\0';
		printf("Received: %s", buffer);

		// reply to the client
		char replyText[] = "Packet received!";
		strncpy(buffer, replyText, strlen(replyText) + 1);
		//printf("Replying with: %s\n", buffer);
		printf("NO Replying : %s\n", buffer);
		//send(clientSocket, buffer, strlen(replyText), 0);
	}
	// close the connection to the client before exit
	close(clientSocket);
	/*
	 * TODO: Delete all client information from "clientSockets" vector.
	 */
	exit (EXIT_SUCCESS);
}

void childProcessListener() {
	if (0 == pid) {
		// we are the child process
		// close the connection on the listening socket
		// (it's being used by the parent process)
		close(serverSocket);
		pthread_t threadReceiver;
		char *message = "Thread for Receiver";
		int iret;
		iret = pthread_create(&threadReceiver, NULL, dataReceiver,
				(void*) message);
		pthread_join(threadReceiver, NULL);
	} else {
		// we are in the parent process

		// close the connection to the client
		// (it's being used by the child process)
		close(clientSocket);
	}
}

void connectionAccepting() {
	// it's important to specify this size first
	clientDataLength = sizeof(clientData);
	isServing = true;
		while (isServing) {
			printf("Starting while loop for accept conn.\n");
		// accept an incoming connection request
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientData,
				&clientDataLength);
		assert(clientSocket > 0); // ensure we have a valid socket
		char *clientAddress = inet_ntoa(clientData.sin_addr);
		int clientPort = ntohs(clientData.sin_port);
		printf("Accepted Connection from: %s:%d\n", clientAddress, clientPort);
				/*
		 * TODO before fork() take to global std::vector<int> connected client
		 * socket parameters.
		 * And ask it from dataSender() function.
		 */
		// fork off into another process
		// this creates another process with the identical program state
		// except that pid == 0 in the child process
		clientSockets.push_back(clientSocket);
		pid = fork();
		childProcessListener();

		if (0 < pid) {
			printf("pid > 0 (forking).\n");
			pthread_t threadSender;
			char *message = "Thread for Sender";
			int iret;
			iret = pthread_create(&threadSender, NULL, dataSender,
					(void*) message);
			pthread_join(threadSender, NULL);
		}
	}
}

/*
 * Open a TCP/IP (stream) socket
 * (and save the descriptor so we can refer to it in the future)
 */
void openSocet() {
	// we're using the internet protocol family and the TCP/IP protocol
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	// bind the socket to the address
	printf("Binding socket\n");
	bindResult = bind(serverSocket, (struct sockaddr *) &serverData,
			sizeof(serverData));
	if (bindResult < 0) {
		printf("Error: Unable to bind socket");
		close(serverSocket);
	} else if (listen(serverSocket, QUEUE_SIZE) < 0) {
		// listening for incoming connections failed
		printf("Error: Unable to listen");
		close(serverSocket);
	} else {
		connectionAccepting();
	}
}

/*
 * Set up the server
 */
void serverSetup() {
	bzero(&serverData, sizeof(serverData));
	// use the Internet Address Family (IPv4)
	serverData.sin_family = AF_INET;
	// accept connections from a client on any address
	serverData.sin_addr.s_addr = htonl(INADDR_ANY);
	// set the port for incoming packets
	serverData.sin_port = htons( PORT);
}

// Entry point 
int main() {
	// Set up the server
	serverSetup();
	// Open TCP/IP (stream) socket
	openSocet();
	return 0;
}

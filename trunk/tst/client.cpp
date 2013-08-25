#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
// threads
#include <pthread.h>
// port number to use
#define PORT 53008
// size of the buffer for incoming data
#define BUF_SIZE 1024
// get the host address from the first commandline argument
char *address;
// descriptor number for the socket we'll use
int serverSocket;
// socket internet address data for the connection to the server
struct sockaddr_in serverData;
// host data
struct hostent *host;
// buffer for sending and receiving data
char buffer[BUF_SIZE];
bool isConnected;
int bytesReceived;

void* dataSender() {
	printf("Main dataSender\n");
	while (isConnected) {
		// send some data
		printf("send > ");
		fgets(buffer, BUF_SIZE, stdin);

		send(serverSocket, buffer, strlen(buffer), 0);

		// wait to receive data from the server
		bytesReceived = recv(serverSocket, buffer, BUF_SIZE, 0);
		// terminate the bytes as a string and print the result
		buffer[bytesReceived] = '\0';
		printf("Received response from sending : %s\n", buffer);
	}
	// it must already closed by dataReceiver() function
//	   close( serverSocket );
}

void* dataReceiver(void *ptr) {
	while (isConnected) {
		// wait to receive data from the server
		printf("DataReceiver Thread wait to receive data from the Server\n");
		bytesReceived = recv(serverSocket, buffer, BUF_SIZE, 0);
		// terminate the bytes as a string and print the result
		buffer[bytesReceived] = '\0';
		printf("Received new massige from server: %s\n", buffer);
	}
	close(serverSocket);
}
/*
 * Open a TCP/IP socket to the server
 * (and save the descriptor so we can refer to it in the future)
 */
void openingSocet() {
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// connect to the server
	printf("Connecting to the Server\n");
	if (connect(serverSocket, (struct sockaddr *) &serverData,
			sizeof(serverData)) < 0) {
		printf("Error connecting\n");
		close(serverSocket);
	} else {
		isConnected = true;

		pthread_t threadReceiver;
		char *message = "Thread for Receiver";
		int iret;
		iret = pthread_create(&threadReceiver, NULL, dataReceiver,
				(void*) message);
		printf("Starting dataReceiver() function with new thread\n");
		pthread_join(threadReceiver, NULL);
		// not necessary divide from main thread.
		printf("Main Thread ->dataSender()\n");
		dataSender();
	}
}
/*
 * Set up the connection to a server
 */
void connectionSetup() {
	bzero(&serverData, sizeof(serverData));
	// use the Internet Address Family (IPv4)
	serverData.sin_family = AF_INET;
	// get host data from the host address
	host = gethostbyname(address);
	// copy the address data from the host struct over to the server address struct
	bcopy(host->h_addr, &(serverData.sin_addr.s_addr), host->h_length);
	// set the port to connect to
	serverData.sin_port = htons( PORT);
}

// Entry point .
int main(int argc, char *argv[]) {
	// Checking arguments
	if (argc != 2) {
		printf("Usage: %s <IP Address>\n", argv[0]);
		exit(0);
	}
	// Set server IP.
	address = argv[1];
	// Set connection cofiguration, parameters.
	connectionSetup();
	// Open socket to the server(TCP/IP).
	openingSocet();

	return 0;
}


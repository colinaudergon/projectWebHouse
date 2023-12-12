/******************************************************************************/
/** \file       startup.c
 *******************************************************************************
 *
 *  \brief      Main application for the Rasberry-Pi webhouse
 *
 *  \author     fue1
 *
 *  \date       November 2021
 *
 *  \remark     Last Modification
 *               \li fue1, November 2021, Created
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              main
 *
 *  functions  local:
 *              shutdownHook
 *
 *  Autor	   Elham Firouzi
 *
 ******************************************************************************/

typedef int int32_t;

//----- Header-Files -----------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include "jansson.h"
#include "Webhouse.h"
#include "handshake.h"
#include <string.h>

//----- Macros -----------------------------------------------------------------
#define TRUE 1
#define FALSE 0
#define TX_BUFFER_SIZE 100

//----- Function prototypes ----------------------------------------------------
static void shutdownHook(int32_t sig);
void initSocket(void);
void sendDataTCP(const char *message);
void closeConnection(void);

//----- Data -------------------------------------------------------------------
static volatile int eShutdown = FALSE;
const SERVER_PORT_NBR = 8000;

//----- Implementation ---------------------------------------------------------

/*******************************************************************************
 *  function :    main
 ******************************************************************************/
/** \brief     void clear_rxBuffer(void)   Starts the socket server (ip: localhost, port:5000) and waits
 *                on a connection attempt of the client.
 *
 *  \type         global
 *
 *  \return       EXIT_SUCCESS
 *
 ******************************************************************************/
struct sockaddr_in server;
struct sockaddr_in client;
int bind_status;
int listen_status;
int server_sock_id;
int com_sock_id;
int addrlen = sizeof(struct sockaddr_in);
struct sockaddr_in addr_remote;
int backlog = 5;
int newSock_id;
char txBuf[TX_BUFFER_SIZE];
int tx_msg_len;

int main(int argc, char **argv)
{
	signal(SIGINT, shutdownHook);

	printf("Init Webhouse\r\n");
	initWebhouse();
	fflush(stdout);

	printf("Main Loop\n");
	initSocket();
	while (eShutdown == FALSE)
	{
		fflush(stdout);
		// printf("Connected");
		sleep(1);
	}

	closeWebhouse();
	printf("Close Webhouse\n");
	fflush(stdout);

	return EXIT_SUCCESS;
}

void initSocket(void)
{
	int socket_desc, new_socket, c;
	struct sockaddr_in server, client;

	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		printf("Could not create socket");

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT_NBR);

	// Bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
		puts("bind failed");
	puts("bind done");

	// Listen
	listen(socket_desc, 3);

	// Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
	if (new_socket < 0)
		perror("accept failed");

	puts("Connection accepted");
	return;
}
/*******************************************************************************
 *  function :    sendDataTCP
 ******************************************************************************/
void sendDataTCP(const char *message)
{
	// Copy the provided message into txBuf
	// strncpy(txBuf, message, TX_BUFFER_SIZE);
	txBuf[0] = "a";
	// Send the data
	tx_msg_len = send(newSock_id, &txBuf[0], TX_BUFFER_SIZE, 0);

	// Check for errors or connection loss
	printf("%d", tx_msg_len);
	if (tx_msg_len > 0)
	{
		/* Data was sent */
		printf("Data sent\n");
	}
	else if (tx_msg_len == 0)
	{
		/* Connection was lost */
		printf("Connection lost\n");
	}
	else
	{
		/* Data not sent */
		printf("Data not sent\n");
	}
}

void closeConnection(void)
{
	close(newSock_id);
}

/*******************************************************************************
 *  function :    shutdownHook
 ******************************************************************************/
/** \brief        Handle the registered signals (SIGTERM, SIGINT)
 *
 *  \type         static
 *
 *  \param[in]    sig    incoming signal
 *
 *  \return       void
 *
 ******************************************************************************/
static void shutdownHook(int32_t sig)
{
	printf("\n ");
	printf("Ctrl-C pressed....shutdown hook in main\n");
	fflush(stdout);
	eShutdown = TRUE;
}

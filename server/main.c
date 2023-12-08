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

	initWebhouse();
	printf("Init Webhouse\r\n");
	fflush(stdout);
	initSocket();

	while (eShutdown == FALSE)
	{
		printf("Main Loop\n");
		fflush(stdout);
		const char *message = "Hello world";
        sendDataTCP(message);
		sleep(1);
	}

	closeWebhouse();
	printf("Close Webhouse\n");
	fflush(stdout);

	return EXIT_SUCCESS;
}

void initSocket(void)
{

	// Calls binding
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT_NBR);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// Calls socket
	server_sock_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock_id > 0)
		;
	else
	{
		printf("Error: socket could not be openedr\r\n");
		return -1;
	}

	bind_status = bind(server_sock_id, (struct sockaddr *)&server, addrlen);
	if (bind_status < 0)
	{
		close(server_sock_id);
	}
	else
	{
		/* Socket bound to desired port */
		printf("Socket bound to desired port");
		listen_status = listen(server_sock_id, backlog);
		if (listen_status > 0)
		{
			newSock_id = accept(server_sock_id, (struct sockaddr *)&client,
								&addrlen);
			if (newSock_id < 0)
			{
				close(newSock_id);
				printf("Failed to accept socket");
			}
		}
	}
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
	printf("Ctrl-C pressed....shutdown hook in main\n");
	fflush(stdout);
	eShutdown = TRUE;
}

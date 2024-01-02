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
 * //modified 02.01 11.42
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              main
 *
 *  functions  local:
 *              shutdownHook
 *
 *  Autor          Elham Firouzi
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
#include <sys/select.h>

#include "jansson.h"
#include "Webhouse.h"
#include "handshake.h"

//----- Macros -----------------------------------------------------------------
#define TRUE 1
#define FALSE 0
#define SERVER_PORT_NBR 8000
#define RX_BUFFER_SIZE 1024

//----- Function prototypes ----------------------------------------------------
static void shutdownHook(int32_t sig);

//----- Data -------------------------------------------------------------------
static volatile int eShutdown = FALSE;

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
int main(int argc, char **argv)
{
	int bind_status;
	int listen_status;
	int server_sock_id;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int addrlen = sizeof(struct sockaddr_in);
	int addrlen_remote = 0;
	int backlog = 5;

	signal(SIGINT, shutdownHook);

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT_NBR);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	initWebhouse();
	printf("Init Webhouse\r\n");
	fflush(stdout);

	server_sock_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock_id < 0)
	{
		perror("Error: socket could not be opened");
		return -1;
	}

	printf("Socket created succesfully\n");

	bind_status = bind(server_sock_id, (struct sockaddr *)&server, addrlen);
	if (bind_status < 0)
	{
		perror("Error: socket could not be bound");
		close(server_sock_id);
		return -1;
	}

	listen_status = listen(server_sock_id, backlog);
	if (listen_status < 0)
	{
		perror("Error: failed to listen");
		close(server_sock_id);
		return -1;
	}
	printf("Succesfully listened\n");
	while (eShutdown == FALSE)
	{
		// printf("Main Loop\n");
		fflush(stdout);

		// fd_set readfds;
		// FD_ZERO(&readfds);
		// FD_SET(server_sock_id, &readfds);

		// struct timeval timeout;
		// timeout.tv_sec = 0;
		// timeout.tv_usec = 10000; // 10 milliseconds

		// int ready = select(server_sock_id + 1, &readfds, NULL, NULL, &timeout);
		// if (ready < 0)
		// {
		// 	perror("Error in select.");
		// 	break;
		// }
		// else if (ready > 0)
		// {
			int com_sock_id = accept(server_sock_id, (struct sockaddr *)&client, &addrlen_remote);
			if (com_sock_id < 0)
			{
				perror("Error accepting connection");
			}
			else
			{
				printf("Accepted connection\n");
				char rxBuf[RX_BUFFER_SIZE];

				/* Connection established, use newSock_id to communicate with client */
				for (;;)
				{
					int rx_data_len = recv(com_sock_id, (void *)rxBuf, RX_BUFFER_SIZE, MSG_DONTWAIT);
					if (rx_data_len > 0)
					{
						rxBuf[rx_data_len] = '\0'; // Is the message a handshake request
						if (strncmp(rxBuf, "GET", 3) == 0)
						{ // Yes -> create the handshake response and send it back
							char response[WS_HS_ACCLEN];
							get_handshake_response(rxBuf, response);
							send(com_sock_id, (void *)response, strlen(response), 0);
							printf("Handshake ok\n");
						}
						else if (rx_data_len == 0)
						{
							printf("Client closed the connection\n");
							// close(com_sock_id);
							break;
						}
						/* No -> decode incoming message, process the command and send back an acknowledge message */
						else
						{
							char command[rx_data_len];
							decode_incoming_request(rxBuf, command);
							command[strlen(command)] = '\0';
							// processCommand(command);
							char response[] = "<Command executed>";
							char codedResponse[strlen(response) + 2];
							code_outgoing_response(response, codedResponse);
							printf("com_sock_id: %d\n", com_sock_id);
							printf("response: %s\n", response);
							send(com_sock_id, (void *)codedResponse, strlen(codedResponse), 0);
						}
						if (eShutdown == TRUE)
						{
							break;
						}
					}
				}
				close(com_sock_id);
			}

			usleep(10000);
			// sleep(1);
		// }
	}
	closeWebhouse();
	printf("Close Webhouse\n");
	fflush(stdout);

	return EXIT_SUCCESS;
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

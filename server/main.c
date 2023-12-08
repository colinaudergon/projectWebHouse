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

//----- Macros -----------------------------------------------------------------
#define TRUE 1
#define FALSE 0

//----- Function prototypes ----------------------------------------------------
static void shutdownHook (int32_t sig);

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
int main(int argc, char **argv) {
	int bind_status;
	int listen_status;
	int server_sock_id;
	int com_sock_id;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int addrlen = sizeof (struct sockaddr_in);
	int backlog = 5;
	
	signal(SIGINT, shutdownHook);

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT_NBR);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	initWebhouse();
    printf("Init Webhouse\r\n");
	fflush(stdout);

	server_sock_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server_sock_id > 0);
	else{
		printf("Error: socket could not be openedr\r\n");
		return -1;
	}

	bind_status = bind(server_sock_id, (struct sockaddr *)&server, addrlen);
	if(bind_status > 0);
	else{
		close(server_sock_id);
		printf("Error: socket could not be bound\r\n");
		return -1;
	}

	listen_status = listen(server_sock_id, backlog);
	if(listen_status > 0);
	else{
		close(server_sock_id);
		printf("Error: failed to listen\r\n");
	}



	while (eShutdown == FALSE) {
		printf("Main Loop\n");
		fflush(stdout);
		
		//usleep(1000);
		sleep(1);
	}

	closeWebhouse();
	printf ("Close Webhouse\n");
	fflush (stdout);

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
static void shutdownHook(int32_t sig) {
    printf("Ctrl-C pressed....shutdown hook in main\n");
    fflush(stdout);
    eShutdown = TRUE;
}

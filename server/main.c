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

#include "jsmn.h"
#include "Webhouse.h"
#include "handshake.h"

//----- Macros -----------------------------------------------------------------
#define TRUE 1
#define FALSE 0
#define SERVER_PORT_NBR 8000
#define RX_BUFFER_SIZE 1024

#define TV 170
#define RL 158
#define SL 159
#define L1 125
#define L2 126
#define HE 141
#define TE 153
#define AA 130
#define READ 82
#define WRITE 87

//----- Function prototypes ----------------------------------------------------
static void shutdownHook(int32_t sig);
static int processCommand(char *input, jsmntok_t *tokens);
int extractSubstring(char *target, char *input, int start, int end, int maxsize);
static void unbindSocket(int socket_fd);

//----- Data -------------------------------------------------------------------
static volatile int eShutdown = FALSE;

//----- Command Processing -----------------------------------------------------
int parsing_result;
jsmn_parser parser;
jsmntok_t tokens[8];

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

	// json parser init
	jsmn_init(&parser);

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

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(server_sock_id, &readfds);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000; // 10 milliseconds

		int ready = select(server_sock_id + 1, &readfds, NULL, NULL, &timeout);
		if (ready < 0)
		{
			perror("Error in select.");
			break;
		}
		else if (ready > 0)
		{
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
						/* No -> decode incoming message, process the command and send back an acknowledge message */
						else
						{

							char command[rx_data_len];
							char response[100];
							decode_incoming_request(rxBuf, command); // RxBuffer is garbage here,and command is "empty"
							command[strlen(command)] = '\0';
							printf("Command: %s\n", command);
							printf("rxBuffer: %s\n", rxBuf);
							int rProcessCommand = 0;

							parsing_result = jsmn_parse(&parser, command, strlen(command), tokens, 8);
							printf("Parsing result: %d\n", parsing_result);
							switch (parsing_result)
							{
							case JSMN_ERROR_INVAL:
								printf("Error: bad token, JSON string is corrupted\r\n");
								break;
							case JSMN_ERROR_NOMEM:
								printf("Error: not enough tokens, JSON string is too large\r\n");
								break;
							case JSMN_ERROR_PART:
								printf("Error: JSON string is too short, expecting more JSON data\r\n");
								break;
							default:
								rProcessCommand = processCommand(command, tokens);
								break;
							}

							printf("rProcess value: %d\n", rProcessCommand);
							if (rProcessCommand == 0)
							{
								strcpy(response, "<Write command executed successfully>");
							}
							else if (rProcessCommand > 0)
							{
								sprintf(response, "<Read command executed successfully: val = %d>", rProcessCommand);
							}
							else
							{
								strcpy(response, "<Command failed>");
							}

							char codedResponse[strlen(response) + 2];
							code_outgoing_response(response, codedResponse);
							printf("com_sock_id: %d\n", com_sock_id);
							printf("response: %s\n", response);
							printf("\n\n");
							send(com_sock_id, (void *)codedResponse, strlen(codedResponse), 0);
							// free(response);
						}
					}
					//Do we keep this?
					// 09/01 colin
					// Test alarm state and send if it has detected something:
					if (getAlarmState() == 1)
					{
						char response[100];
						char codedResponse[strlen(response) + 2];
						strcpy(response, "<Alarm detected something>");
						code_outgoing_response(response, codedResponse);
						send(com_sock_id, (void *)codedResponse, strlen(codedResponse), 0);
						printf("\n\n");
					}
					if (eShutdown == TRUE)
					{
						break;
					}
				}
				close(com_sock_id);
				unbindSocket(server_sock_id);
			}
			usleep(10000);
			// sleep(1);
		}
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

/*******************************************************************************
 *  function :    processCommand
 ******************************************************************************/
/** \brief      Handle the received command
 *
 *  \type       static
 *
 *  \param[in]  tokens Tokens after parsing the
 *
 *  \return		0 when write command executed successfully
 * 				>= 1 when read command executed successfully
 * 				< 0 when command failed
 *
 ******************************************************************************/

static int processCommand(char *input, jsmntok_t *tokens)
{

	int num_tokens = 2 * tokens[0].size;
	char substrings[num_tokens][5];

	// Extract substrings
	int i = 0;
	// printf("value of num_token: %d\n",num_tokens);
	int extractResult;
	while (i < num_tokens)
	{
		extractResult = extractSubstring(&substrings[i][0], input, tokens[i + 1].start, tokens[i + 1].end, 5);
		if (extractResult > 0)
		{
			i++;
		}
	}

	// printf("Substring 0: %s\n", substrings[0]);
	// printf("Substring 2: %s\n", substrings[2]);
	// printf("Substring 4: %s\n", substrings[4]);

	if (strcmp(substrings[0], "cmd") != 0)
	{
		return -1;
	}
	if (strcmp(substrings[2], "dev") != 0)
	{
		return -1;
	}
	if (strcmp(substrings[4], "val") != 0)
	{
		return -1;
	}

	int cmd_num = (int)substrings[1][0];
	int dev_num = (int)substrings[3][0] + (int)substrings[3][1];

	// ToDo: correct for ascii code code
	//  int val_num = 10 * (int)substrings[5][0] + (int)substrings[5][1] - 11 * '0';
	//  int val_num = 10 * ((int)substrings[5][0]-'0') + (int)substrings[5][1]-'0';
	int val_num = ((int)substrings[5][0] - '0');
	printf("Val num: %d\n", val_num);
	//  if (val_num < 0 || val_num > 99)
	//  {
	//  	printf("Did not passed the validation");
	//  	return -1;
	//  }

	// Change process command -> read/write output are biased
	switch (cmd_num)
	{
	case READ:
		switch (dev_num)
		{
		case TV:
			return 1 + getTVState();
			break;
		case L1:
			return 1 + getLED1State();
			break;
		case L2:
			return 1 + getLED2State();
			break;
		case TE:
			return 1 + getTemp();
			break;
		case HE:
			return 1 + getHeatState();
			break;
		case AA:
		//Do we keep this?
			return 1 + getAlarmState();
			break;
		}
		break;

	case WRITE:
		switch (dev_num)
		{
		case TV:
			if (val_num == 0)
				turnTVOff();
			else
				turnTVOn();
			return 0;
			break;
		case RL:
			dimRLamp(val_num);
			return 0;
			break;
		case SL:
			dimSLamp(val_num);
			return 0;
			break;
		case L1:
			if (val_num == 0)
				turnLED1Off();
			else
				turnLED1On();
			return 0;
			break;
		case L2:
			if (val_num == 0)
				turnLED2Off();
			else
				turnLED2On();
			return 0;
			break;
		case HE:
			if (val_num == 0)
				turnHeatOff();
			else
				turnHeatOn();
			return 0;
			break;
		}
		break;
	default:
		printf("Neither Read or Write commad: error");
		break;
	}
}

// extractSubstring(&substrings[i][0], input, tokens[i + 1].start, tokens[i + 1].end, 5) > 0)
int extractSubstring(char *target, char *input, int start, int end, int maxsize)
{
	int i = 0;
	if (start < 0)
	{
		return -1;
	}
	else if (end < 0 || end <= start)
	{
		return -1;
	}
	else if (end - start > maxsize)
	{
		return -1;
	}
	else
	{
		i = start;
		while (i < end)
		{
			target[i - start] = input[i];
			i++;
		}
		target[i - start] = '\0';
	}
	// printf("Target result: %s\n", target);
	// printf("value of I from exctract substring: %d\n",i);
	return i;
}
// Function to unbind the address
static void unbindSocket(int socket_fd)
{
	struct sockaddr_in server;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	// Get the current socket address
	if (getsockname(socket_fd, (struct sockaddr *)&server, &addrlen) == 0)
	{
		printf("Unbinding address %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
		memset(&server, 0, sizeof(struct sockaddr_in));
		server.sin_family = AF_UNSPEC;
		bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	}
}
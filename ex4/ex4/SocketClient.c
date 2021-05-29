/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
#pragma warning (disable : 4996)

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "SharedInformation.h"
#include "SocketSendRecieve.h"
#include "SocketClient.h"
#include "Board.h"
#include "Messages.h"
#include "LinkedList.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_MESSAGE_LENGTH 150
#define BOARD_WIDTH  7
#define BOARD_HEIGHT 6
#define MAX_USERNAME_LENGTH 31
#define ERROR_CODE_THREAD 0x555
#define PLAY_MESSAGE "play "
#define MESSAGE_MESSAGE "message "
#define EXIT_MESSAGE "exit"
#define MAX_CLIENTS 2
#define DONT_EXIT -2

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

FILE* fpLog = NULL;
FILE* fpInput = NULL;
SOCKET m_socket;
LinkedList* bufferForInput = NULL;
LinkedList* lastStringInBuffer = NULL;
BOOL isHuman = true;
BOOL isOurTurn = false;
BOOL wasGameStarted = false;
char* username = NULL;
static HANDLE mutexForBuffer;
static HANDLE mutexForLogFile;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/*
The function writes for the log file.
parameters: newLog holds the log we want to write to the log file.
*/
static void WriteToLogFile(char *newLog);

/*
The function gets input from the user.
parameters: username holds the username that plays on this thread.
return 0 if succeed, ERROR_CODE if failed.
*/
static DWORD GetDataThreadFromUser(char* username);

/*
The function prints the board.
parameters: AcceptedStr holds the board status as a string.
			hConsole holds the socket for the client.
*/
static void PrintBoardClient(char *AcceptedStr, HANDLE hConsole);

/*
The function prints a message on screen.
parameters: The message (on a prefix & rest-of-message format).
*/
static void PrintMessageOnScreen(char* prefix, char* message);

/*
The function execute the next step the user required.
parameters: messageID holds the ID of the step (as described on Messages.h)
			AcceptedStr holds the message we want to execute.
			hConsole holds the socket for the player.
			log holds the relevant log for the log file.
return 0 if the player wants to quit, and DONT_EXIT otherwise.
*/
static int ExecuteNextStep(int messageID, char *AcceptedStr, HANDLE hConsole, char* log);

/*
The function sends messages from the buffer to the server.
*/
static DWORD SendDataThread(void);

/*
The function gets messages from the server.
*/
static DWORD RecvDataThread(void);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void WriteToLogFile(char *newLog)
{
	DWORD wait_code;
	if (newLog == NULL)
	{
		printf("Exit from program: got newLog as NULL\n");
		exit(ERROR_CODE);
	}
	wait_code = WaitForSingleObject(mutexForLogFile, INFINITE);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("Waiting for thread failed. Ending program\n");
		exit(ERROR_CODE);
	}
	fprintf(fpLog, "%s", newLog);
	if (newLog[strlen(newLog) - 1] != '\n')
		fprintf(fpLog, "\n");
	if (ReleaseMutex(mutexForLogFile) == false)
	{
		printf("Couldn't release mutex. Ending program\n");
		exit(ERROR_CODE);
	}
}

static DWORD GetDataThreadFromUser(char* username)
{
	DWORD wait_code;
	char* tempStringToSend = NULL;
	char* rawMessageFromUser = NULL;
	if (username == NULL)
	{
		printf("Exit from program: got username as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got username as NULL\n");
		exit(ERROR_CODE);
	}
	wait_code = WaitForSingleObject(mutexForBuffer, INFINITE);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("Waiting for thread failed. Ending program\n");
		WriteToLogFile("Custom message: Waiting for thread failed. Ending program\n");
		exit(ERROR_CODE);
	}
	tempStringToSend = (char*)malloc(MAX_MESSAGE_LENGTH);
	if (tempStringToSend == NULL)
	{
		printf("Exit from program: got tempStringToSend as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got tempStringToSend as NULL\n");
		exit(ERROR_CODE);
	}
	if (CreateNewUserRequestMessage(&tempStringToSend, username) == ERROR_CODE)
	{
		printf("Exit from program: CreateNewUserRequestMessage failed\n");
		WriteToLogFile("Custom message: Exit from program: CreateNewUserRequestMessage failed\n");
		exit(ERROR_CODE);
	}
	if (AddNode(&lastStringInBuffer, tempStringToSend) == ERROR_CODE)
	{
		printf("Exit from program: AddNode failed\n");
		WriteToLogFile("Custom message: Exit from program: AddNode failed\n");
		exit(ERROR_CODE);
	}
	free(tempStringToSend);
	if (ReleaseMutex(mutexForBuffer) == false)
	{
		printf("Couldn't release mutex. Ending program\n");
		WriteToLogFile("Custom message: Couldn't release mutex. Ending program\n");
		exit(ERROR_CODE);
	}

	while (1)
	{
		rawMessageFromUser = (char *)malloc(MAX_MESSAGE_LENGTH);
		if (rawMessageFromUser == NULL)
		{
			printf("Exit from program: Memory allocation failed\n");
			WriteToLogFile("Custom message: Exit from program: Memory allocation failed\n");
			exit(ERROR_CODE);
		}
		if (isHuman == true)
		{
			fgets(rawMessageFromUser, MAX_MESSAGE_LENGTH, stdin);
		}
		else
		{
			while (wasGameStarted == false || isOurTurn == false);
			fgets(rawMessageFromUser, MAX_MESSAGE_LENGTH, fpInput);
			isOurTurn = false;
		}
		wait_code = WaitForSingleObject(mutexForBuffer, INFINITE);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("Waiting for thread failed. Ending program\n");
			WriteToLogFile("Custom message: Waiting for thread failed. Ending program\n");
			exit(ERROR_CODE);
		}
		tempStringToSend = (char*)malloc(MAX_MESSAGE_LENGTH);
		if (tempStringToSend == NULL)
		{
			printf("Exit from program: Memory allocation failed\n");
			WriteToLogFile("Custom message: Exit from program: Memory allocation failed\n");
			exit(ERROR_CODE);
		}
		if (strncmp(rawMessageFromUser, PLAY_MESSAGE, strlen(PLAY_MESSAGE)) == 0
			&& strlen(rawMessageFromUser) == strlen(PLAY_MESSAGE) + 2
			&& isdigit(rawMessageFromUser[strlen(PLAY_MESSAGE)]))
		{
			if (CreateNewMoveMessage(&tempStringToSend, rawMessageFromUser) == ERROR_CODE)
			{
				printf("Exit from program: CreateNewMoveMessage failed\n");
				WriteToLogFile("Custom message: Exit from program: CreateNewMoveMessage failed\n");
				exit(ERROR_CODE);
			}
			if (AddNode(&lastStringInBuffer, tempStringToSend) == ERROR_CODE)
			{
				printf("Exit from program: AddNode failed\n");
				WriteToLogFile("Custom message: Exit from program: AddNode failed\n");
				exit(ERROR_CODE);
			}
		}
		else if (strncmp(rawMessageFromUser, MESSAGE_MESSAGE, strlen(MESSAGE_MESSAGE)) == 0
			&& strlen(rawMessageFromUser) >= strlen(EXIT_MESSAGE) + 1)
		{
			if (CreateChatMessage(&tempStringToSend, rawMessageFromUser) == ERROR_CODE)
			{
				printf("Exit from program: CreateChatMessage failed\n");
				WriteToLogFile("Custom message: Exit from program: CreateChatMessage failed\n");
				exit(ERROR_CODE);
			}
			if (AddNode(&lastStringInBuffer, tempStringToSend) == ERROR_CODE)
			{
				printf("Exit from program: AddNode failed\n");
				WriteToLogFile("Custom message: Exit from program: AddNode failed\n");
				exit(ERROR_CODE);
			}
			isOurTurn = true;
		}
		else if (strncmp(rawMessageFromUser, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0 
			&& strlen(rawMessageFromUser) == strlen(EXIT_MESSAGE) + 1)
			return 0;
		else
		{
			printf("Error: Illegal command\n");
			WriteToLogFile("Error: Illegal command\n");
			isOurTurn = true;
		}
		if (ReleaseMutex(mutexForBuffer) == false)
		{
			printf("Couldn't release mutex. Ending program\n");
			WriteToLogFile("Custom message: Couldn't release mutex. Ending program\n");
			exit(ERROR_CODE);
		}
		free(tempStringToSend);
		free(rawMessageFromUser);
	}
}

static void PrintBoardClient(char *AcceptedStr, HANDLE hConsole)
{
	if (AcceptedStr == NULL)
	{
		printf("Exit from program: got PrintBoardClient parameters as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got PrintBoardClient parameters as NULL\n");
		exit(ERROR_CODE);
	}
	printf("\n");
	if (PrintBoard(AcceptedStr + strlen(BOARD_VIEW), hConsole) == ERROR_CODE)
	{
		printf("Exit from program: PrintBoard failed\n");
		WriteToLogFile("Custom message: Exit from program: PrintBoard failed\n");
		exit(ERROR_CODE);
	}
	printf("\n");
}

//Reading data coming from the server

static void PrintMessageOnScreen(char* prefix, char* message)
{
	if (prefix == NULL || message == NULL)
	{
		printf("Exit from program: got PrintMessageOnScreen parameters as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got PrintMessageOnScreen parameters as NULL\n");
		exit(ERROR_CODE);
	}
	printf("%s", prefix);
	while (*message != '\0' && *message != '\n')
	{
		if (*message != ';')
			printf("%c", *message);
		message++;
	}
	printf("\n");
}

static int ExecuteNextStep(int messageID, char *AcceptedStr, HANDLE hConsole, char* log)
{
	int i;
	char *chatMessagePrefix = NULL;
	char *AcceptedStrPtr = NULL;
	if (AcceptedStr == NULL || log == NULL)
	{
		printf("Exit from program: got ExecuteNextStep parameters as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got ExecuteNextStep parameters as NULL\n");
		exit(ERROR_CODE);
	}
	if (messageID == NEW_USER_DECLINED_ID)
	{
		printf("Request to join was refused");
		return ERROR_CODE;
	}
	else if (messageID == GAME_STARTED_ID)
	{
		printf("Game is on!\n");
		wasGameStarted = true;
	}
	else if (messageID == GAME_ENDED_ID)
	{
		if (strcmp(GAME_ENDED, AcceptedStr) == 0)
		{
			WriteToLogFile("Game ended. Everybody wins!\n");
			printf("Game ended. Everybody wins!\n");
		}
		else
		{
			strcpy(log, "Game ended. The winner is ");
			strcat(log, AcceptedStr + strlen(GAME_ENDED));
			strcat(log, "!\n");
			WriteToLogFile(log);
			printf("%s", log);
		}
		free(AcceptedStr);
		return 0; //"quit" signals an exit from the client side
	}
	else if (messageID == BOARD_VIEW_ID)
		PrintBoardClient(AcceptedStr, hConsole);
	else if (messageID == PLAY_ACCEPTED_ID)
		PrintMessageOnScreen("", "Well played");
	else if (messageID == PLAY_DECLINED_ID)
	{
		PrintMessageOnScreen("Error: ", AcceptedStr + strlen(PLAY_DECLINED));
		if (strcmp(AcceptedStr + strlen(PLAY_DECLINED), "Illegal; ;move") == 0)
			isOurTurn = true;
	}
	else if (messageID == TURN_SWITCH_ID)
	{
		strcpy(log, AcceptedStr + strlen(GAME_ENDED) + 1);
		strcat(log, "'s turn\n");
		WriteToLogFile(log);
		printf("%s", log);
		if (strcmp(username, AcceptedStr + strlen(GAME_ENDED) + 1) == 0 && isOurTurn == false)
			isOurTurn = true;
	}
	else if (messageID == RECIEVE_MESSAGE_ID)
	{
		AcceptedStrPtr = AcceptedStr;
		chatMessagePrefix = (char*)malloc(MAX_USERNAME_LENGTH);
		if (chatMessagePrefix == NULL)
		{
			printf("Exit from program: could not allocate memory\n");
			WriteToLogFile("Custom message: Exit from program: could not allocate memory\n");
			exit(ERROR_CODE);
		}
		AcceptedStrPtr = AcceptedStrPtr + strlen(RECIEVE_MESSAGE);
		i = 0;
		while (AcceptedStrPtr[i] != ';')
		{
			chatMessagePrefix[i] = AcceptedStrPtr[i];
			i++;
		}
		chatMessagePrefix[i] = ':';
		chatMessagePrefix[i + 1] = '\0';
		AcceptedStrPtr = AcceptedStrPtr + i + 1;
		PrintMessageOnScreen(chatMessagePrefix, AcceptedStrPtr);
		free(chatMessagePrefix);
	}
	return DONT_EXIT;
}

static DWORD RecvDataThread(void)
{
	int errorCode;
	TransferResult_t RecvResC;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int messageID;
	char* log = NULL;
	while (1)
	{
		char *AcceptedStr = NULL;
		RecvResC = ReceiveString(&AcceptedStr, m_socket);
		log = (char *)malloc(MAX_MESSAGE_LENGTH);
		if (log == NULL)
		{
			printf("Exit from program: could not allocate memory\n");
			WriteToLogFile("Custom message: Exit from program: could not allocate memory\n");
			exit(ERROR_CODE);
		}
		if (RecvResC == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
			exit(ERROR_CODE);
		}
		else if (RecvResC == TRNS_DISCONNECTED)
		{
			printf("Server closed connection. Bye!\n");
			return 0;
		}
		strcpy(log , "Received from server: ");
		strcat(log, AcceptedStr);
		WriteToLogFile(log);
		messageID = GetMessageID(AcceptedStr);
		if (messageID == ERROR_CODE)
		{
			printf("Exit from program: GetMessageID failed\n");
			WriteToLogFile("Custom message: Exit from program: GetMessageID failed\n");
			exit(ERROR_CODE);
		}
		errorCode = ExecuteNextStep(messageID, AcceptedStr, hConsole, log);
		if (errorCode != DONT_EXIT)
			return errorCode;
		free(AcceptedStr);
		free(log);
		log = NULL;
	}

	close(hConsole);
	return 0;
}

static DWORD SendDataThread(void)
{
	TransferResult_t SendRes;
	DWORD wait_code;
	char* log = NULL;
	while (1)
	{
		while (bufferForInput->nextString == NULL);
		wait_code = WaitForSingleObject(mutexForBuffer, INFINITE);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("Waiting for thread failed. Ending program\n");
			WriteToLogFile("Custom message: Waiting for thread failed. Ending program\n");
			exit(ERROR_CODE);
		}
		
		if (STRINGS_ARE_EQUAL(bufferForInput->nextString->string, "exit"))
			return ERROR_CODE; //"quit" signals an exit from the client side

		SendRes = SendString(bufferForInput->nextString->string, m_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
			exit(ERROR_CODE);
		}
		log = (char*)malloc(MAX_MESSAGE_LENGTH);
		if (log == NULL)
		{
			printf("Exit from program: could not allocate memory\n");
			WriteToLogFile("Custom message: Exit from program: could not allocate memory\n");
			exit(ERROR_CODE);
		}
		strcpy(log, "Sent to server: ");
		strcat(log, bufferForInput->nextString->string);
		WriteToLogFile(log);
		if (ClearNode(&bufferForInput, &lastStringInBuffer) == ERROR_CODE)
		{
			printf("Exit from program: ClearNode failed\n");
			WriteToLogFile("Custom message: Exit from program: ClearNode failed\n");
			exit(ERROR_CODE);
		}
		if (ReleaseMutex(mutexForBuffer) == false)
		{
			printf("Couldn't release mutex. Ending program\n");
			WriteToLogFile("Custom message: Couldn't release mutex. Ending program\n");
			exit(ERROR_CODE);
		}
		free(log);
		log = NULL;
	}
}

bool MainClient(int argc, char* argv[])
{
	int i;
	SOCKADDR_IN clientService;
	HANDLE hThread[3];
	DWORD wait_code;
	char* logPath = argv[2];
	int port = atoi(argv[3]);
	char* inputMode = argv[4];
	char* inputFilePath;
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	fopen_s(&fpLog, logPath, "w");
	if (fpLog == NULL)
	{
		printf("Exit from program: couldn't create log file\n");
		exit(ERROR_CODE);
	}

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		WriteToLogFile("Custom message: Error at WSAStartup()\n");
	}
	if (strcmp(inputMode, "file") == 0)
	{
		isHuman = false;
		inputFilePath = argv[5];
		fpInput = fopen(inputFilePath, "r");
		if (fpInput == NULL)
		{
			printf("Exit from program: got fpInput as NULL\n");
			WriteToLogFile("Custom message: Exit from program: got fpInput as NULL\n");
			exit(ERROR_CODE);
		}
	}
	username = (char *)malloc(MAX_USERNAME_LENGTH);
	if (username == NULL)
	{
		printf("Exit from program: Could not allocate memory\n");
		WriteToLogFile("Custom message: Exit from program: Could not allocate memory\n");
		exit(ERROR_CODE);
	}
	mutexForBuffer = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL); /* un-named */
	mutexForLogFile = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL); /* un-named */
	if (mutexForBuffer == NULL || mutexForLogFile == NULL)
	{
		printf("Exit from program: Could not create mutex\n");
		WriteToLogFile("Custom message: Exit from program: Could not create mutex\n");
		exit(ERROR_CODE);
	}
	bufferForInput = NewLinkedList();
	if (bufferForInput == NULL)
	{
		printf("Exit from program: got bufferForInput as NULL\n");
		WriteToLogFile("Custom message: Exit from program: got bufferForInput as NULL\n");
		exit(ERROR_CODE);
	}
	lastStringInBuffer = bufferForInput;
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WriteToLogFile("Custom message: socket error\n");
		WSACleanup();
		return false;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	if (clientService.sin_addr.s_addr == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		WriteToLogFile("Custom message: The string cannot be converted into an ip address. ending program.\n");
		exit(ERROR_CODE);
	}
	clientService.sin_port = htons(port); //Setting the port to connect to.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on 127.0.0.1:%d\n", port);
		wait_code = WaitForSingleObject(mutexForLogFile, INFINITE);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("Waiting for thread failed. Ending program\n");
			exit(ERROR_CODE);
		}
		fprintf(fpLog, "Failed connecting to server on 127.0.0.1:%d\n", port);
		if (ReleaseMutex(mutexForLogFile) == false)
		{
			printf("Couldn't release mutex. Ending program\n");
			WriteToLogFile("Custom message: Couldn't release mutex. Ending program\n");
			exit(ERROR_CODE);
		}
		WSACleanup();
		return false;
	}
	printf("Connected to server on 127.0.0.1:%d\n", port);
	wait_code = WaitForSingleObject(mutexForLogFile, INFINITE);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("Waiting for thread failed. Ending program\n");
		exit(ERROR_CODE);
	}
	fprintf(fpLog, "Connected to server on 127.0.0.1:%d\n", port);
	if (ReleaseMutex(mutexForLogFile) == false)
	{
		WriteToLogFile("Custom message: Couldn't release mutex. Ending program\n");
		printf("Couldn't release mutex. Ending program\n");
		exit(ERROR_CODE);
	}
	if (isHuman == true)
	{
		printf("Enter username\n");
		gets(username);
	}
	else
	{
		fgets(username, MAX_USERNAME_LENGTH, fpInput);
		username[strlen(username) - 1] = '\0';
	}
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);
	hThread[2] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)GetDataThreadFromUser,
		username,
		0,
		NULL											
	);
	WaitForMultipleObjects(3, hThread, FALSE, INFINITE);
	for (i = 0; i <= MAX_CLIENTS; i++)
	{
		TerminateThread(hThread[i], ERROR_CODE_THREAD);
		CloseHandle(hThread[i]);
	}
	CloseHandle(mutexForBuffer);
	CloseHandle(mutexForLogFile);
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
	free(username);
	fclose(fpLog);
	if (isHuman == false)
		fclose(fpInput);
	WSACleanup();
	return true;
}

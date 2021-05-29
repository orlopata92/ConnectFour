/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <Windows.h>

#include "SharedInformation.h"
#include "SocketSendRecieve.h"
#include "SocketServer.h"
#include "Board.h"
#include "Messages.h"
#include "LinkedList.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define NUM_OF_WORKER_THREADS 2
#define MAX_USERNAME_LENGTH 31
#define MAX_MESSAGE_LENGTH 150

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

HANDLE ThreadHandles[2*NUM_OF_WORKER_THREADS];
static HANDLE mutexForLogFileServer;
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
BOOL gameOver = false;
BOOL wasGameStartedServer = false;
int numberOfPlayers;
int currentPlayerServer;
int boardServer[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
char usernames[2][MAX_USERNAME_LENGTH] = { 0 };
char* pathLogServer = NULL;
FILE* fpLogServer = NULL;
static HANDLE mutexForBuffer[2];
LinkedList* bufferForInputArray[2] = { 0 };
LinkedList* lastStringInBufferArray[2] = { 0 };
int numberOfThreadThatExited = -1;
BOOL needToCloseHandles = false;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/*
The function writes for the log file.
parameters: newLog holds the log we want to write to the log file.
*/
static void WriteToLogFile(char *newLog);

/*
The function execute steps for the first username recieved.
parameters: newUsername holds the new username
			AcceptSocket holds the socket for the client
			stringToSend holds the string we want to send to the user.
*/
static void InitFirstUsername(char* newUsername, SOCKET AcceptSocket, char* stringToSend);

/*
The function execute steps for the first username recieved.
parameters: newUsername holds the new username
			AcceptSocket holds the socket for the client
			stringToSend holds the string we want to send to the user.
*/
static void InitSecondUsername(char* newUsername, SOCKET AcceptSocket, char* stringToSend, SOCKET MainSocket);

/*
The function cleanup resources.
*/
static void ServerCleanup3();

/*
The function send update for the client about the new state of the board.
parameters: boardServer holds the board as a 2D int array
			t_socket holds the socket between the server and the client.
*/
static void SendBoardUpdate(int boardServer[BOARD_HEIGHT][BOARD_WIDTH], SOCKET *t_socket);

/*
The function gets messages from the server.
*/
static DWORD RecvThread(SOCKET *t_socket);

/*
Service thread is the thread that opens for each successful client connection and "talks" to the client.
parameters: t_socket holds the socket between the server and the client.
The function returns 0 for a successfull run.
*/
static DWORD ServiceThread(SOCKET *t_socket);

/*
This function runs the steps for ending the game.
*/
static void EndGameFunction();

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void WriteToLogFile(char *newLog)
{
	DWORD wait_code;
	if (newLog == NULL)
	{
		printf("Exit from program: got newLog as NULL\n");
		exit(ERROR_CODE);
	}
	wait_code = WaitForSingleObject(mutexForLogFileServer, INFINITE);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("Waiting for thread failed. Ending program\n");
		exit(ERROR_CODE);
	}
	fprintf(fpLogServer, "%s", newLog);
	if (newLog[strlen(newLog) - 1] != '\n')
		fprintf(fpLogServer, "\n");
	if (ReleaseMutex(mutexForLogFileServer) == false)
	{
		printf("Couldn't release mutex. Ending program\n");
		exit(ERROR_CODE);
	}
}

static void InitFirstUsername(char* newUsername, SOCKET AcceptSocket, char* stringToSend)
{
	TransferResult_t SendRes;
	int currentRow, currentCol;
	if (newUsername == NULL || stringToSend == NULL)
	{
		printf("Exit from program: InitFirstUsername got NULL as input\n");
		WriteToLogFile("Custom message: Exit from program: InitFirstUsername got NULL as input\n");
		exit(ERROR_CODE);
	}
	strcpy_s(usernames[0], MAX_USERNAME_LENGTH, newUsername + 17);
	if (CreateNewUserAcceptedMessage(0, &stringToSend) == ERROR_CODE)
	{
		WriteToLogFile("Custom message: Exit from program: CreateNewUserAcceptedMessage failed\n");
		printf("Exit from program: CreateNewUserAcceptedMessage failed\n");
		exit(ERROR_CODE);
	}
	numberOfPlayers++;
	SendRes = SendString(stringToSend, AcceptSocket);
	if (SendRes == TRNS_FAILED)
	{
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	for (currentRow = 0; currentRow < BOARD_HEIGHT; currentRow++)
		for (currentCol = 0; currentCol < BOARD_WIDTH; currentCol++)
			boardServer[currentRow][currentCol] = 0;
	ThreadInputs[0] = AcceptSocket; // shallow copy: don't close
									  // AcceptSocket, instead close
									  // ThreadInputs[Ind] when the
									  // time comes.
	ThreadHandles[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ServiceThread,
		&(ThreadInputs[0]),
		0,
		NULL
	);
	ThreadHandles[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvThread,
		&(ThreadInputs[0]),
		0,
		NULL
	);
}

static void InitSecondUsername(char* newUsername, SOCKET AcceptSocket, char* stringToSend, SOCKET MainSocket)
{
	TransferResult_t recvResB, SendRes;
	if (newUsername == NULL || stringToSend == NULL)
	{
		WriteToLogFile("Custom message: Exit from program: InitSecondUsername got NULL as input\n");
		printf("Exit from program: InitSecondUsername got NULL as input\n");
		exit(ERROR_CODE);
	}
	while (strcmp(usernames[0], newUsername + 17) == 0)
	{
		SendRes = SendString(NEW_USER_DECLINED, AcceptSocket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
			exit(ERROR_CODE);
		}
		closesocket(AcceptSocket);
		AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			WriteToLogFile("Custom message: Accepting connection with client failed, error %ld\n", WSAGetLastError());
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			exit(ERROR_CODE);
		}
		newUsername = NULL;
		recvResB = ReceiveString(&newUsername, AcceptSocket);
		if (recvResB == TRNS_FAILED)
		{
			WriteToLogFile("Custom message: Service socket error while reading, closing thread.\n");
			printf("Service socket error while reading, closing thread.\n");
			closesocket(AcceptSocket);
			exit(ERROR_CODE);
		}
	}
	strcpy_s(usernames[1], MAX_USERNAME_LENGTH, newUsername + 17);
	if (CreateNewUserAcceptedMessage(1, &stringToSend) == ERROR_CODE)
	{
		WriteToLogFile("Custom message: Exit from program: CreateNewUserAcceptedMessage failed\n");
		printf("Exit from program: CreateNewUserAcceptedMessage failed\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(stringToSend, AcceptSocket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	ThreadInputs[1] = AcceptSocket; // shallow copy: don't close 
								  // AcceptSocket, instead close 
								  // ThreadInputs[Ind] when the
								  // time comes.
	SendRes = SendString(GAME_STARTED, ThreadInputs[0]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(GAME_STARTED, ThreadInputs[1]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	SendBoardUpdate(boardServer, &(ThreadInputs[0]));
	SendBoardUpdate(boardServer, &(ThreadInputs[1]));
	currentPlayerServer = 1;
	if (CreateTurnSwitchMessage(usernames[currentPlayerServer - 1], &stringToSend) == ERROR_CODE)
	{
		WriteToLogFile("Custom message: Exit from program: CreateTurnSwitchMessage failed\n");
		printf("Exit from program: CreateTurnSwitchMessage failed\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(stringToSend, ThreadInputs[0]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(stringToSend, ThreadInputs[1]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	numberOfPlayers++;
	ThreadHandles[2] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ServiceThread,
		&(ThreadInputs[1]),
		0,
		NULL
	);
	ThreadHandles[3] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvThread,
		&(ThreadInputs[1]),
		0,
		NULL
	);
	wasGameStartedServer = true;
}

static void ServerCleanup3()
{
	TransferResult_t SendRes;
	TerminateThread(ThreadHandles[0], 0);
	TerminateThread(ThreadHandles[1], 0);
	TerminateThread(ThreadHandles[2], 0);
	CloseHandle(ThreadHandles[0]);
	CloseHandle(ThreadHandles[1]);
	CloseHandle(ThreadHandles[2]);
	closesocket(ThreadInputs[0]);
	closesocket(ThreadInputs[1]);
	SendRes = SendString(GAME_ENDED, ThreadInputs[0]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(GAME_ENDED, ThreadInputs[1]);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	closesocket(ThreadInputs[0]);
	closesocket(ThreadInputs[1]);
}

bool MainServer(char* argv[])
{
	int Ind;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	pathLogServer = argv[2];
	int port = atoi(argv[3]);
	numberOfPlayers = 0;
	char* newUsername = NULL;
	TransferResult_t recvResB, SendRes;
	char* stringToSend = NULL;
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);	
	fpLogServer = fopen(pathLogServer, "w");
	if (fpLogServer == NULL)
	{
		printf("Exit from program: Couldn't open file for writing\n");
		exit(ERROR_CODE);
	}
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		WriteToLogFile("Custom message: error at WSAStartup( ), ending program.\n");
		return false;
	}
	stringToSend = (char *)malloc(MAX_MESSAGE_LENGTH);
	if (stringToSend == NULL)
	{
		printf("Exit from program: could not allocate memory \n");
		WriteToLogFile("Custom message: Exit from program: could not allocate memory \n");
		exit(ERROR_CODE);
	}
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		WriteToLogFile("Custom message: Error at socket\n");
		goto server_cleanup_1;
	}
	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		WriteToLogFile("Custom message: The string cannot be converted into an ip address. ending program.\n");
		goto server_cleanup_2;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(port); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		WriteToLogFile("Custom message: bind( ) failed. Ending program\n");
		goto server_cleanup_2;
	}
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		WriteToLogFile("Custom message: Failed listening on socket.\n");
		goto server_cleanup_2;
	}
	mutexForLogFileServer = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL); /* un-named */
	if (mutexForLogFileServer == NULL)
	{
		printf("Exit from program: Could not create mutex\n");
		WriteToLogFile("Custom message: Exit from program: Could not create mutex\n");
		exit(ERROR_CODE);
	}


	printf("Waiting for a client to connect...\n");
	while (1)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			WriteToLogFile("Custom message: Accepting connection with client failed\n");
			goto server_cleanup_3;
		}
		newUsername = NULL;
		recvResB = ReceiveString(&newUsername, AcceptSocket);
		if (recvResB == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			WriteToLogFile("Custom message: Service socket error while reading, closing thread.\n");
			closesocket(AcceptSocket);
			return ERROR_CODE;
		}
		if (wasGameStartedServer == true)
		{
			SendRes = SendString(NEW_USER_DECLINED, AcceptSocket);
			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
				exit(ERROR_CODE);
			}
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			if (needToCloseHandles == true)
			{
				for (int i = 0; i < 2 * NUM_OF_WORKER_THREADS; i++)
					CloseHandle(ThreadHandles[i]);
			}
			for (int i = 0; i < NUM_OF_WORKER_THREADS; i++)
			{
				mutexForBuffer[i] = CreateMutex(
					NULL,   /* default security attributes */
					FALSE,	/* don't lock mutex immediately */
					NULL); /* un-named */
				if (mutexForBuffer[i] == NULL)
				{
					printf("Exit from program: Could not create mutex\n");
					WriteToLogFile("Custom message: Exit from program: Could not create mutex\n");
					exit(ERROR_CODE);
				}
				
				bufferForInputArray[i] = NewLinkedList();
				if (bufferForInputArray[i] == NULL)
				{
					printf("Exit from program: NewLinkedList failed\n");
					WriteToLogFile("Custom message: Exit from program: NewLinkedList failed\n");
					exit(ERROR_CODE);
				}
				lastStringInBufferArray[i] = bufferForInputArray[i];
				ThreadHandles[2 * i] = NULL;
				ThreadHandles[2 * i + 1] = NULL;
			}
			
			InitFirstUsername(newUsername, AcceptSocket, stringToSend);
			AcceptSocket = accept(MainSocket, NULL, NULL);
			if (AcceptSocket == INVALID_SOCKET)
			{
				printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
				WriteToLogFile("Custom message: Accepting connection with client failed\n");
				goto server_cleanup_3;
			}
			newUsername = NULL;
			recvResB = ReceiveString(&newUsername, AcceptSocket);
			if (recvResB == TRNS_FAILED)
			{
				printf("Service socket error while reading, closing thread.\n");
				WriteToLogFile("Custom message: Service socket error while reading, closing thread.\n");
				closesocket(AcceptSocket);
				return ERROR_CODE;
			}
			InitSecondUsername(newUsername, AcceptSocket, stringToSend, MainSocket);
		}
	} 
server_cleanup_3:
	ServerCleanup3();
server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
	free(stringToSend);
	fclose(fpLogServer);
	return false;
}

static void SendBoardUpdate(int boardServer[BOARD_HEIGHT][BOARD_WIDTH], SOCKET *t_socket)
{
	TransferResult_t SendRes;
	char* sendStr = NULL;
	sendStr = malloc(MAX_MESSAGE_LENGTH);
	if (sendStr == NULL)
	{
		printf("Exit from program: could not allocate memory\n");
		WriteToLogFile("Custom message: Exit from program: could not allocate memory\n");
		exit(ERROR_CODE);
	}
	if (CreateBoardViewMessage(&sendStr, boardServer) == ERROR_CODE)
	{
		printf("Exit from program: CreateBoardViewMessage failed\n");
		WriteToLogFile("Custom message: Exit from program: CreateBoardViewMessage failed\n");
		exit(ERROR_CODE);
	}
	if (t_socket == NULL)
	{
		printf("Exit from program: SendBoardUpdate got NULL as input\n");
		WriteToLogFile("Custom message: Exit from program: SendBoardUpdate got NULL as input\n");
		exit(ERROR_CODE);
	}
	SendRes = SendString(sendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
		exit(ERROR_CODE);
	}
	free(sendStr);
}

static DWORD RecvThread(SOCKET *t_socket)
{
	DWORD wait_code;
	TransferResult_t recvResB;
	char *AcceptedStr = NULL;
	int myPlayerNumber;
	if (t_socket == NULL)
	{
		printf("Exit from program: RecvThread got NULL as input\n");
		WriteToLogFile("Custom message: Exit from program: RecvThread got NULL as input\n");
		exit(ERROR_CODE);
	}
	if ((*t_socket) == ThreadInputs[0])
		myPlayerNumber = 1;
	else
		myPlayerNumber = 2;
	while (1)
	{
		AcceptedStr = NULL;
		recvResB = ReceiveString(&AcceptedStr, *t_socket);
		if (recvResB == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			WriteToLogFile("Custom message: Service socket error while reading, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		else if (recvResB == TRNS_DISCONNECTED)
		{
			printf("Player disconnected. Ending communication.\n");
			WriteToLogFile("Custom message: Player disconnected. Ending communication.\n");
			fprintf(fpLogServer, "Player disconnected. Ending communication.\n");
			gameOver = true;
			if (*t_socket == ThreadInputs[0])
				numberOfThreadThatExited = 1;
			else
				numberOfThreadThatExited = 3;
			EndGameFunction();
			gameOver = false;
			wasGameStartedServer = false;
			return 1;
		}

		wait_code = WaitForSingleObject(mutexForBuffer[myPlayerNumber-1], INFINITE);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("Waiting for thread failed. Ending program\n");
			WriteToLogFile("Custom message: Waiting for thread failed. Ending program\n");
			exit(ERROR_CODE);
		}
		if (AddNode(&(lastStringInBufferArray[myPlayerNumber - 1]), AcceptedStr) == ERROR_CODE)
		{
			printf("Exit from program: AddNode failed\n");
			WriteToLogFile("Custom message: Exit from program: AddNode failed\n");
			exit(ERROR_CODE);
		}
		if (ReleaseMutex(mutexForBuffer[myPlayerNumber - 1]) == ERROR_CODE)
		{
			printf("Exit from program: could not release mutex\n");
			WriteToLogFile("Custom message: Exit from program: could not release mutex\n");
			exit(ERROR_CODE);
		}
	}
}

static DWORD ServiceThread(SOCKET *t_socket)
{
	int playerNumber;
	int newRow;
	int newCol;
	int endFlag = false;
	DWORD wait_code;
	char* stringToSend = NULL;
	TransferResult_t SendRes;
	stringToSend = (char*)malloc(MAX_MESSAGE_LENGTH);
	if (stringToSend == NULL)
	{

		printf("Exit from program: could not allocate memory \n");
		WriteToLogFile("Custom message: Exit from program: could not allocate memory \n");
		exit(ERROR_CODE);
	}
	if (t_socket == NULL)
	{
		printf("Exit from program: ServiceThread got NULL as input\n");
		WriteToLogFile("Custom message: Exit from program: ServiceThread got NULL as input\n");
		exit(ERROR_CODE);
	}
	if (*t_socket == ThreadInputs[0])
		playerNumber= 1;
	else
		playerNumber = 2;
	while (gameOver == false)
	{
		char *AcceptedStr = NULL;
		AcceptedStr = (char *)malloc(MAX_MESSAGE_LENGTH);
		if (AcceptedStr == NULL)
		{
			printf("Exit from program: could not allocate memory \n");
			WriteToLogFile("Custom message: Exit from program: could not allocate memory \n");
			exit(ERROR_CODE);
		}
		while (bufferForInputArray[playerNumber-1]->nextString == NULL);
		wait_code = WaitForSingleObject(mutexForBuffer[playerNumber-1], INFINITE);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("Waiting for thread failed. Ending program\n");
			WriteToLogFile("Custom message: Waiting for thread failed. Ending program\n");
			exit(ERROR_CODE);
		}
		strcpy(AcceptedStr, bufferForInputArray[playerNumber-1]->nextString->string);
		if (ClearNode(&(bufferForInputArray[playerNumber - 1]), &(lastStringInBufferArray[playerNumber - 1])) == ERROR_CODE)
		{
			printf("Exit from program: ClearNode failed\n");
			WriteToLogFile("Custom message: Exit from program: ClearNode failed\n");
			exit(ERROR_CODE);
		}
		if (ReleaseMutex(mutexForBuffer[playerNumber - 1]) == ERROR_CODE)
		{
			printf("Exit from program: ReleaseMutex failed\n");
			WriteToLogFile("Custom message: Exit from program: ReleaseMutex failed\n");
			exit(ERROR_CODE);
		}
		if (wasGameStartedServer == false)
		{
			strcpy(stringToSend, PLAY_DECLINED);
			strcat(stringToSend, "Game; ;has; ;not; ;started");
			SendRes = SendString(stringToSend, *t_socket);
			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
				exit(ERROR_CODE);
			}
		}
		else if (GetMessageID(AcceptedStr) == SEND_MESSAGE_ID)
		{
			if (CreateRecieveMessage(&stringToSend, usernames[playerNumber - 1], AcceptedStr) == ERROR_CODE)
			{
				printf("Exit from program: CreateRecieveMessage failed\n");
				WriteToLogFile("Custom message: Exit from program: CreateRecieveMessage failed\n");
				exit(ERROR_CODE);
			}
			SendRes = SendString(stringToSend, ThreadInputs[3 - playerNumber - 1]);
			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
				exit(ERROR_CODE);
			}
		}
		else if (GetMessageID(AcceptedStr) == PLAY_REQUEST_ID)
		{
			newCol = atoi(&(AcceptedStr[strlen(PLAY_REQUEST)]));
			newRow = FindRowForNewTurn(newCol, boardServer);
			if (newRow != -1 && currentPlayerServer == playerNumber)
			{
				SendRes = SendString(PLAY_ACCEPTED, *t_socket);
				if (SendRes == TRNS_FAILED)
				{
					printf("Socket error while trying to write data to socket\n");
					WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
					exit(ERROR_CODE);
				}
				if (FillBoard(&(boardServer[newRow][newCol]), currentPlayerServer) == ERROR_CODE)
				{
					printf("Exit from program: FillBoard failed\n");
					WriteToLogFile("Custom message: Exit from program: FillBoard failed\n");
					exit(ERROR_CODE);
				}
				SendBoardUpdate(boardServer, &(ThreadInputs[0]));
				SendBoardUpdate(boardServer, &(ThreadInputs[1]));
				if (IsGameOver(boardServer, newRow, newCol, currentPlayerServer) == true)
				{
					gameOver = true;
					if (CreateGameEndedMessage(usernames[currentPlayerServer - 1], &stringToSend) == ERROR_CODE)
					{
						printf("Exit from program: CreateGameEndedMessage failed\n");
						WriteToLogFile("Custom message: Exit from program: CreateGameEndedMessage failed\n");
						exit(ERROR_CODE);
					}
				}
				else if (IsTie(boardServer) == true)
				{
					gameOver = true;
					if (CreateGameEndedMessage(NULL, &stringToSend) == ERROR_CODE)
					{
						printf("Exit from program: CreateGameEndedMessage failed\n");
						WriteToLogFile("Custom message: Exit from program: CreateGameEndedMessage failed\n");
						exit(ERROR_CODE);
					}
				}
				if (gameOver == false)
				{
					currentPlayerServer = 3 - currentPlayerServer;
					if (CreateTurnSwitchMessage(usernames[currentPlayerServer - 1], &stringToSend) == ERROR_CODE)
					{
						printf("Exit from program: CreateTurnSwitchMessage failed\n");
						WriteToLogFile("Custom message: Exit from program: CreateTurnSwitchMessage failed\n");
						exit(ERROR_CODE);
					}
					SendRes = SendString(stringToSend, ThreadInputs[0]);
					if (SendRes == TRNS_FAILED)
					{
						printf("Socket error while trying to write data to socket\n");
						WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
						exit(ERROR_CODE);
					}
					SendRes = SendString(stringToSend, ThreadInputs[1]);
					if (SendRes == TRNS_FAILED)
					{
						printf("Socket error while trying to write data to socket\n");
						WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
						exit(ERROR_CODE);
					}
				}
				else
				{
					SendRes = SendString(stringToSend, ThreadInputs[0]);
					if (SendRes == TRNS_FAILED)
					{
						printf("Socket error while trying to write data to socket\n");
						WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
						exit(ERROR_CODE);
					}
					SendRes = SendString(stringToSend, ThreadInputs[1]);
					if (SendRes == TRNS_FAILED)
					{
						printf("Socket error while trying to write data to socket\n");
						WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
						exit(ERROR_CODE);
					}
					numberOfThreadThatExited = (playerNumber-1) * 2;
					EndGameFunction();
				}
			}
			else
			{
				strcpy(stringToSend, PLAY_DECLINED);
				if (currentPlayerServer != playerNumber)
					strcat(stringToSend, "Not; ;your; ;turn");
				else if (newRow == -1)
					strcat(stringToSend, "Illegal; ;move");
				SendRes = SendString(stringToSend, *t_socket);
				if (SendRes == TRNS_FAILED)
				{
					printf("Socket error while trying to write data to socket\n");
					WriteToLogFile("Custom message: Socket error while trying to write data to socket\n");
					exit(ERROR_CODE);
				}
			}
			if (gameOver == true)
			{
				gameOver = false;
				wasGameStartedServer = false;
			}
		}
		else
		{
			printf("Exit from program: GetMessageID failed\n");
			WriteToLogFile("Custom message: Exit from program: GetMessageID failed\n");
			exit(ERROR_CODE);
		}
		free(AcceptedStr);
	}
	free(stringToSend);
	return 0;
}

static void EndGameFunction()
{
	for (int i = 0; i < 2 * NUM_OF_WORKER_THREADS; i++)
		if (i != numberOfThreadThatExited)
			TerminateThread(ThreadHandles[i], 0x555);
	needToCloseHandles = true;
	shutdown(ThreadInputs[0], SD_BOTH);
	shutdown(ThreadInputs[1], SD_BOTH);
	closesocket(ThreadInputs[0]);
	closesocket(ThreadInputs[1]);
	numberOfPlayers = 0;
	CloseHandle(mutexForBuffer[0]);
	CloseHandle(mutexForBuffer[1]);
	printf("Waiting for a client to connect...\n");
}

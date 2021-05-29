#include <string.h>
#include <stdlib.h>
#include "Board.h"
#include "Messages.h"
#include "SharedInformation.h"

#define MAX_MESSAGE_LENGTH 150

int CreateRecieveMessage(char** stringToSend, char* whoSent, char* originalMessage)
{
	if (stringToSend == NULL || *stringToSend == NULL || whoSent == NULL || originalMessage == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, sizeof(RECIEVE_MESSAGE) ,RECIEVE_MESSAGE);
	strcat_s(*stringToSend, MAX_MESSAGE_LENGTH , whoSent);
	(*stringToSend)[strlen(*stringToSend) + 1] = '\0';
	(*stringToSend)[strlen(*stringToSend)] = ';';
	strcat_s(*stringToSend, MAX_MESSAGE_LENGTH , originalMessage + strlen(SEND_MESSAGE));
	return 0;
}

int CreateChatMessage(char** stringToSend, char* rawMessageFromUser)
{
	int rawMessageIndex = strlen("message ");
	int stringToSendIndex = strlen(SEND_MESSAGE);
	if (stringToSend == NULL || *stringToSend == NULL || rawMessageFromUser == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, sizeof(SEND_MESSAGE), SEND_MESSAGE);
	while (rawMessageFromUser[rawMessageIndex] != '\0')
	{
		while (rawMessageFromUser[rawMessageIndex] == ' ' && rawMessageFromUser[rawMessageIndex] != '\0')
		{
			(*stringToSend)[stringToSendIndex] = ' ';
			stringToSendIndex++;
			rawMessageIndex++;
			if (rawMessageFromUser[rawMessageIndex] != ' ' && rawMessageFromUser[rawMessageIndex] != '\0')
			{
				(*stringToSend)[stringToSendIndex] = ';';
				stringToSendIndex++;
			}
		}
		while (rawMessageFromUser[rawMessageIndex] != ' ' && rawMessageFromUser[rawMessageIndex] != '\0')
		{
			(*stringToSend)[stringToSendIndex] = rawMessageFromUser[rawMessageIndex];
			stringToSendIndex++;
			rawMessageIndex++;
			if (rawMessageFromUser[rawMessageIndex] == ' ' && rawMessageFromUser[rawMessageIndex] != '\0')
			{
				(*stringToSend)[stringToSendIndex] = ';';
				stringToSendIndex++;
			}
		}
	}
	(*stringToSend)[stringToSendIndex] = '\0';
	return 0;
}

int CreateNewMoveMessage(char** stringToSend, char* rawMessageFromUser)
{
	if (stringToSend == NULL || *stringToSend == NULL || rawMessageFromUser == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, sizeof(PLAY_REQUEST), PLAY_REQUEST);
	strncat_s(*stringToSend, MAX_MESSAGE_LENGTH, rawMessageFromUser + strlen("play "), MAX_MESSAGE_LENGTH);
	return 0;
}

int CreateNewUserRequestMessage(char** stringToSend, char* username)
{
	if (stringToSend == NULL || *stringToSend == NULL || username == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, MAX_MESSAGE_LENGTH, NEW_USER_REQUEST);
	strcat_s(*stringToSend, MAX_MESSAGE_LENGTH, username);
	return 0;
}

int CreateBoardViewMessage(char** stringToSend, int boardServer[BOARD_HEIGHT][BOARD_WIDTH])
{
	int i, j, k=0;
	char* boardAsString = NULL;
	boardAsString = (char*)malloc(MAX_MESSAGE_LENGTH);
	if (boardAsString == NULL || stringToSend == NULL || *stringToSend == NULL)
		return ERROR_CODE;
	for (i = BOARD_HEIGHT - 1; i >= 0; i--)
	{
		for (j = 0; j < BOARD_WIDTH; j++)
		{
			*(boardAsString + k) = boardServer[i][j] + '0';
			k++;
		}
		if (i > 0)
		{
			*(boardAsString + k) = ';';
			k++;
		}
	}
	*(boardAsString + k) = '\0';
	strcpy_s(*stringToSend, MAX_MESSAGE_LENGTH, BOARD_VIEW);
	strcat_s(*stringToSend, MAX_MESSAGE_LENGTH , boardAsString);
	free(boardAsString);
	return 0;
}

int CreateGameEndedMessage(char* username, char** stringToSend)
{
	if (stringToSend == NULL || (*stringToSend) == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, MAX_MESSAGE_LENGTH, GAME_ENDED);
	if (username != NULL)
		strcat_s(*stringToSend, MAX_MESSAGE_LENGTH, username);
	return 0;
}

int CreateNewUserAcceptedMessage(int numberOfPlayers, char** stringToSend)
{
	if (stringToSend == NULL || (*stringToSend) == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, MAX_MESSAGE_LENGTH, NEW_USER_ACCEPTED);
	(*stringToSend)[strlen(NEW_USER_ACCEPTED) + 1] = '\0';
	(*stringToSend)[strlen(NEW_USER_ACCEPTED)] = numberOfPlayers + '1';
	return 0;
}

int CreateTurnSwitchMessage(char* username, char** stringToSend)
{
	if (username == NULL || stringToSend == NULL || (*stringToSend) == NULL)
		return ERROR_CODE;
	strcpy_s(*stringToSend, MAX_MESSAGE_LENGTH, TURN_SWITCH);
	strcat_s(*stringToSend, MAX_MESSAGE_LENGTH, username);
	return 0;
}

int GetMessageID(char* messageRecv)
{
	if (messageRecv == NULL)
		return ERROR_CODE;
	if (strncmp(messageRecv, NEW_USER_REQUEST, strlen(NEW_USER_REQUEST)) == 0)
		return (int)NEW_USER_REQUEST_ID;
	if (strncmp(messageRecv, NEW_USER_ACCEPTED, strlen(NEW_USER_ACCEPTED)) == 0)
		return (int)NEW_USER_ACCEPTED_ID;
	if (strncmp(messageRecv, NEW_USER_DECLINED, strlen(NEW_USER_DECLINED)) == 0)
		return (int)NEW_USER_DECLINED_ID;
	if (strncmp(messageRecv, GAME_STARTED, strlen(GAME_STARTED)) == 0)
		return (int)GAME_STARTED_ID;
	if (strncmp(messageRecv, BOARD_VIEW, strlen(BOARD_VIEW)) == 0)
		return (int)BOARD_VIEW_ID;
	if (strncmp(messageRecv, TURN_SWITCH, strlen(TURN_SWITCH)) == 0)
		return (int)TURN_SWITCH_ID;
	if (strncmp(messageRecv, PLAY_REQUEST, strlen(PLAY_REQUEST)) == 0)
		return (int)PLAY_REQUEST_ID;
	if (strncmp(messageRecv, PLAY_ACCEPTED, strlen(PLAY_ACCEPTED)) == 0)
		return (int)PLAY_ACCEPTED_ID;
	if (strncmp(messageRecv, PLAY_DECLINED, strlen(PLAY_DECLINED)) == 0)
		return (int)PLAY_DECLINED_ID;
	if (strncmp(messageRecv, GAME_ENDED, strlen(GAME_ENDED)) == 0)
		return (int)GAME_ENDED_ID;
	if (strncmp(messageRecv, SEND_MESSAGE, strlen(SEND_MESSAGE)) == 0)
		return (int)SEND_MESSAGE_ID;
	if (strncmp(messageRecv, RECIEVE_MESSAGE, strlen(RECIEVE_MESSAGE)) == 0)
		return (int)RECIEVE_MESSAGE_ID;
	return ERROR_CODE;
}

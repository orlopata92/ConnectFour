/*
	Authors -	Michal Arad
				Or Lopata
	Description -
	This module helps us to create and manage the messages between the clients & server.
*/

#ifndef MESSAGES_H
#define MESSAGES_H

#define NEW_USER_REQUEST "NEW_USER_REQUEST:"
#define NEW_USER_ACCEPTED "NEW_USER_ACCEPTED:"
#define NEW_USER_DECLINED "NEW_USER_DECLINED"
#define GAME_STARTED "GAME_STARTED"
#define BOARD_VIEW "BOARD_VIEW:"
#define TURN_SWITCH "TURN_SWITCH:"
#define PLAY_REQUEST "PLAY_REQUEST:"
#define PLAY_ACCEPTED "PLAY_ACCEPTED"
#define PLAY_DECLINED "PLAY_DECLINED:"
#define GAME_ENDED "GAME_ENDED:"
#define SEND_MESSAGE "SEND_MESSAGE:"
#define RECIEVE_MESSAGE "RECIEVE_MESSAGE:"
#define NEW_USER_REQUEST_ID 0
#define NEW_USER_ACCEPTED_ID 1
#define NEW_USER_DECLINED_ID 2
#define GAME_STARTED_ID 3
#define BOARD_VIEW_ID 4
#define TURN_SWITCH_ID 5
#define PLAY_REQUEST_ID 6
#define PLAY_ACCEPTED_ID 7
#define PLAY_DECLINED_ID 8
#define GAME_ENDED_ID 9
#define SEND_MESSAGE_ID 10
#define RECIEVE_MESSAGE_ID 11

/*
The function sends the ID of the recieved message.
parameters: messageRecv holds the message.
return the ID of the recieved message (as listed on this file) or ERROR_CODE for failure.
*/
int GetMessageID(char* messageRecv);

/*
The function creates a message for accepted username.
parameters: stringToSend holds the final message (output)
			numberOfPlayers is the number of the players right now.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateNewUserAcceptedMessage(int numberOfPlayers, char** stringToSend);

/*
The function creates a message for ended game.
parameters: stringToSend holds the final message (output)
			username holds the name of the user that won (NULL for tie).
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateGameEndedMessage(char* username, char** stringToSend);

/*
The function creates a message for turn switch.
parameters: stringToSend holds the final message (output)
			username holds the username that will play on the next turn.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateTurnSwitchMessage(char* username, char** stringToSend);

/*
The function creates a message for board view.
parameters: stringToSend holds the final message (output)
			boardServer is a 2D array that represents the board's state.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateBoardViewMessage(char** stringToSend, int boardServer[BOARD_HEIGHT][BOARD_WIDTH]);

/*
The function creates a message for a new user request.
parameters: stringToSend holds the final message (output)
			username is the requested new username.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateNewUserRequestMessage(char** stringToSend, char* username);

/*
The function creates a message for a new move.
parameters: stringToSend holds the final message (output)
			rawMessageFromUser is the exact input from the user.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateNewMoveMessage(char** stringToSend, char* rawMessageFromUser);

/*
The function creates a message for sent chat message.
parameters: stringToSend holds the final message (output)
			rawMessageFromUser is the exact input from the user.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateChatMessage(char** stringToSend, char* rawMessageFromUser);

/*
The function creates a message for recieved chat message.
parameters: stringToSend holds the final message (output)
			whoSent holds the username who sent the message.
			originalMessage holds the message that was recieved.
return 0 if succeed, ERROR_CODE if failed.
*/
int CreateRecieveMessage(char** stringToSend, char* whoSent, char* originalMessage);

#endif

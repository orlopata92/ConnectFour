/*
	Authors -	Michal Arad
				Or Lopata
	Description -
	This module helps us to manage the board
*/

#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <Windows.h>
#define BOARD_WIDTH  7
#define BOARD_HEIGHT 6

/***********************************************************
* This function checks if the game is over.
* Input: A 2D array representing the board
*		 The coordinates of the last turn
*		 The number of the player who played the last turn.
* Output: A boolean value - was the game ended.
************************************************************/
BOOL IsGameOver(int board[][BOARD_WIDTH], int lastMoveRow, int lastMoveCol, int playerNumber);

/***********************************************************
* This function adds a new player coin on a place on board.
* Input: A pointer to a place on the board and player number.
* Output: '0' if the coin was added to the board, '-1' for error.
************************************************************/
int FillBoard(int* placeOnBoard, int player);

/***********************************************************
* This function prints the board, and uses O as the holes.
* The disks are presented by red or yellow backgrounds.
* Input: A 2D array representing the board
* Output: Prints the board, no return value
************************************************************/
int PrintBoard(char* boardAsString, HANDLE hConsole);

/***********************************************************
* This function find the lowest empty row on a specific column.
* Input: A 2D array representing the board, and a column number.
* Output: Returns the lowest empty row. If the column is full - returns (-1)
************************************************************/
int FindRowForNewTurn(int col, int board[BOARD_HEIGHT][BOARD_WIDTH]);

/***********************************************************
* This function checks if the game ended in tie.
* Input: A 2D array representing the board
* Output: Boolean value: was the game ended in tie?
************************************************************/
BOOL IsTie(int board[][BOARD_WIDTH]);

#endif


#include <stdio.h>
#include <Windows.h>
#include "Board.h"
#include "SharedInformation.h"

#define RED_PLAYER 1
#define YELLOW_PLAYER 2

#define BLACK  15
#define RED    204
#define YELLOW 238

BOOL IsTie(int board[][BOARD_WIDTH])
{
	int i;
	int countCoins = 0;
	for (i = 0; i < BOARD_WIDTH; i++)
	{
		if (board[BOARD_HEIGHT - 1][i] != 0)
			countCoins++;
	}
	if (countCoins == BOARD_WIDTH)
		return true;
	return false;
}

BOOL IsGameOver(int board[][BOARD_WIDTH], int lastMoveRow, int lastMoveCol, int playerNumber)
{
	int firstCol, firstRow, i, j, k;
	BOOL flag;

	if (lastMoveCol - 3 < 0)
		firstCol = 0;
	else
		firstCol = lastMoveCol - 3;
	if (lastMoveRow - 3 < 0)
		firstRow = 0;
	else
		firstRow = lastMoveRow - 3;

	//search for 4 in row
	for (j = firstCol ; j + 3 < BOARD_WIDTH ; j++)
	{
		flag = true;
		for (k = 0; k < 4 && flag == true; k++)
		{
			if (board[lastMoveRow][j + k] != playerNumber)
				flag = false;
		}
		if (flag == true)
			return true;
	}
	//search for 4 in col
	for (i = firstRow; i + 3 < BOARD_HEIGHT; i++)
	{
		flag = true;
		for (k = 0; k < 4 && flag == true; k++)
		{
			if (board[i+k][lastMoveCol] != playerNumber)
				flag = false;
		}
		if (flag == true)
			return true;
	}
	//search for 4 in first diag
	for (i = firstRow , j = firstCol ; i + 3 < BOARD_HEIGHT && j + 3 < BOARD_WIDTH ; i++ , j++)
	{
		flag = true;
		for (k = 0; k < 4 && flag == true; k++)
		{
			if (board[i + k][j + k] != playerNumber)
				flag = false;
		}
		if (flag == true)
			return true;
	}
	//search for 4 in second diag
	if (lastMoveRow + 3 < BOARD_HEIGHT)
		firstRow = lastMoveRow + 3;
	else
		firstRow = BOARD_HEIGHT;
	for (i = firstRow, j = firstCol; i - 3 >= 0 && j + 3 < BOARD_WIDTH; i--, j++)
	{
		flag = true;
		for (k = 0; k < 4 && flag == true; k++)
		{
			if (board[i - k][j + k] != playerNumber)
				flag = false;
		}
		if (flag == true)
			return true;
	}
	return false;
}

int FillBoard(int* placeOnBoard, int player)
{
	if (placeOnBoard == NULL)
		return ERROR_CODE;
	*placeOnBoard = player;
	return 0;
}

int PrintBoard(char* boardAsString , HANDLE  hConsole)
{
	int row, column;
	int indexString = 0;
	if (boardAsString == NULL)
		return ERROR_CODE;
	//Draw the board
	for (row = BOARD_HEIGHT - 1 ; row >= 0 ; row--)
	{
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("| ");
			if (boardAsString[indexString] - '0' == RED_PLAYER)
				SetConsoleTextAttribute(hConsole, RED);

			else if (boardAsString[indexString] - '0' == YELLOW_PLAYER)
				SetConsoleTextAttribute(hConsole, YELLOW);

			printf("O");

			SetConsoleTextAttribute(hConsole, BLACK);
			printf(" ");
			indexString++;
		}
		printf("\n");

		//Draw dividing line between the rows
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("----");
		}
		printf("\n");
		if (boardAsString[indexString] != '\0')
			indexString++;
	}
	return 0;
}

int FindRowForNewTurn(int col, int board[BOARD_HEIGHT][BOARD_WIDTH])
{
	int i;
	if (col < 0 || col >= BOARD_WIDTH)
		return -1;
	if (board[BOARD_HEIGHT - 1][col] != 0)
		return -1;
	for (i = BOARD_HEIGHT - 1; i > 0; i--)
	{
		if (board[i - 1][col] != 0)
			return i;
	}
	return 0;
}

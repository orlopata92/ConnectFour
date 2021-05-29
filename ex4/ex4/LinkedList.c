#include <stdio.h>
#include "LinkedList.h"
#include "SharedInformation.h"

/*
The function fills data in a new node.
parameters: newNode is a pointer to a linked list (contains 1 node).
			data is the information we want to init the newNode with.
return 0 if succeed, ERROR_CODE if failed (memory allocation error);.
*/
static int InitNewNode(LinkedList** newNode, char *data);

LinkedList* NewLinkedList()
{
	LinkedList* newList = NULL;
	newList = (LinkedList *)malloc(sizeof(LinkedList));
	if (newList == NULL)
		return NULL;
	newList->string = NULL;
	newList->nextString = NULL;
	newList->prevString = NULL;
	return newList;
}

static int InitNewNode(LinkedList** newNode, char *data)
{
	if (newNode == NULL)
		return ERROR_CODE;
	*newNode = NewLinkedList();
	if (newNode == NULL)
		return ERROR_CODE;
	(*newNode)->string = (char*)malloc(MAX_MESSAGE_LENGTH);
	if ((*newNode)->string == NULL)
		return ERROR_CODE;
	strcpy((*newNode)->string, data);
	return 0;
}

int AddNode(LinkedList** lastStringInBuffer, char* data)
{
	LinkedList* newNode = NULL;
	if (lastStringInBuffer == NULL || data == NULL || (*lastStringInBuffer) == NULL)
		return ERROR_CODE;
	if (InitNewNode(&newNode, data) == ERROR_CODE)
		return ERROR_CODE;
	newNode->nextString = NULL;
	newNode->prevString = *lastStringInBuffer;
	(*lastStringInBuffer)->nextString = newNode;
	*lastStringInBuffer = newNode;
	return 0;
}

int ClearNode(LinkedList** head, LinkedList** lastStringInBuffer)
{
	LinkedList* deletedNode = NULL;
	if (head == NULL || lastStringInBuffer == NULL || (*head) == NULL || (*lastStringInBuffer) == NULL)
	{
		return ERROR_CODE;
	}
	deletedNode = (*head)->nextString;
	if (deletedNode == *lastStringInBuffer)
		(*lastStringInBuffer) = (*lastStringInBuffer)->prevString;
	(*head)->nextString = (*head)->nextString->nextString;
	if ((*head)->nextString != NULL)
		(*head)->nextString->prevString = *head;
	free(deletedNode->string);
	free(deletedNode);
	return 0;
}

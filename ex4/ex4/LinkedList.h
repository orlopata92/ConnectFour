/*
	Authors -	Michal Arad
				Or Lopata
	Description -
	This module helps us to create and manage linked lists
*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct _LinkedList {
	char* string;
	struct _LinkedList* nextString;
	struct _LinkedList* prevString;
}LinkedList;

/*
The function creates a new (empty) linked list and returns it.
*/
LinkedList* NewLinkedList();

/*
The function creates a new node with the requested value and adds it to a list.
parameters: head is the head of the current list.
			data is the data for the new node.
return 0 if succeed, ERROR_CODE if failed.
*/
int AddNode(LinkedList** head, char* data);

/*
The function removes the first 'real' node on the list (the first one is always 0).
parameters: head is the head of the current list.
            lastStringInBuffer is the last node on the list.
return 0 if succeed, ERROR_CODE if failed.
*/
int ClearNode(LinkedList** head, LinkedList** lastStringInBuffer);

#endif


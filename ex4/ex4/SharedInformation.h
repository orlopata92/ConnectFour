/*
	Authors -	Michal Arad
				Or Lopata
				Amnon Drory - From the course "Introduction to Systems Programming" at Tel-Aviv
				University, School of Electrical Engineering, Winter 2011
	Description -
	This module holds information we want to share between the different modules.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define SERVER_ADDRESS_STR "127.0.0.1"
#define MAX_MESSAGE_LENGTH 150
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define ERROR_CODE -1

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_SHARED_H
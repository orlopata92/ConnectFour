/*
	Authors -	Michal Arad
				Or Lopata
				Amnon Drory - From the course "Introduction to Systems Programming" at Tel-Aviv
				University, School of Electrical Engineering, Winter 2011
	Description -
	This module helps us to manage the server.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H

#include <stdbool.h>

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/*
This is the main function for server mode.
parameters: the input arguments
return 0 if ran successfully, ERROR_CODE if failed.
*/
bool MainServer(char* argv[]);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_SERVER_H
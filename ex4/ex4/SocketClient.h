/*
	Authors -	Michal Arad
				Or Lopata
				Amnon Drory - From the course "Introduction to Systems Programming" at Tel-Aviv
				University, School of Electrical Engineering, Winter 2011
	Description -
	This module helps us to manage clients
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H

#include <stdbool.h>

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/*
This is the main function for client mode.
parameters: the input arguments
return 0 if ran successfully, ERROR_CODE if failed.
*/
bool MainClient(int argc, char* argv[]);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_CLIENT_H
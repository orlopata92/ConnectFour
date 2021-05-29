/*
	Authors -	Michal Arad
				Or Lopata
	Project - EX4
*/

#include <stdbool.h>
#include "SocketClient.h"
#include "SocketServer.h"
#include "SharedInformation.h"

int main(int argc, char* argv[]) {
	bool runResult = false;
	if (strcmp(argv[1] , "client") == 0)
		runResult = MainClient(argc, argv);
	else if (strcmp(argv[1], "server") == 0)
		runResult = MainServer(argv);
	if (runResult == true)
		return 0;
	return ERROR_CODE;
}
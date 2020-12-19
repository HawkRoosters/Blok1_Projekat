// TaxiService.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"



int main()
{
	char *clientsLocation = GetMessageFromClient();
	printf("Dobio sam adresu klijenta: %s\n", clientsLocation);
	location clLoc = GetLocation(clientsLocation);
	printf("...sto je lokacija: %d  , %d \n", clLoc.x, clLoc.y);

	getchar();
	//char *driversLocation = GetMessageFromClient();
	//printf("Dobio sam lokaciju vozaca: %s\n", driversLocation);
	//location drvLoc = GetLocation(driversLocation);
//	printf("...sto je lokacija: %d  , %d \n", drvLoc.x, drvLoc.y);

	return 0;
}





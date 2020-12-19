// TaxiDriver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "TCPfunctions.h"


int main()
{

	char messageToSend[] = "Lokacija vozaca: (23,42)";
	ClientRequest(messageToSend);


	getchar();
	return 0;
}

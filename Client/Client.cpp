// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"


int main()
{

    char messageToSend[] = "Molim Vas, jedno vozilo na ...";
	ClientRequest(messageToSend);


	getchar();
	return 0;
}







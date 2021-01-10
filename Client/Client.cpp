// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"


DWORD WINAPI ClientToService(LPVOID lpParam)
{
	ClientsRequest();

	return 0;
}

int main()
{

	DWORD request;
	HANDLE hRequest;


	hRequest = CreateThread(NULL, 0, &ClientToService, NULL, 0, &request);
	CloseHandle(hRequest);


	Sleep(5000);
	//getchar();
	return 0;
}







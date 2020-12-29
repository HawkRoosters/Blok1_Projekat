// TaxiDriver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "TCPfunctions.h"

CRITICAL_SECTION dcs;

DWORD WINAPI DriversAnswer(LPVOID lpParam)
{
	int id = *(int*)lpParam;

	EnterCriticalSection(&dcs);
		ClientRequest(id);
		printf("moj ID je %d . \n", id);
	LeaveCriticalSection(&dcs);

	return 0;
}


int main()
{

	DWORD driverClient1, driverClient2;
	HANDLE hDriver1, hDriver2;

	int id1 = 1;
	int id2 = 2;

	InitializeCriticalSection(&dcs);

//	hDriver1 = CreateThread(NULL, 0, &DriversAnswer, &id1, 0, &driverClient1);
	hDriver2 = CreateThread(NULL, 0, &DriversAnswer, &id2, 0, &driverClient2);

	getchar();


//	CloseHandle(hDriver1);
	CloseHandle(hDriver2);
	DeleteCriticalSection(&dcs);


	getchar();
	return 0;
}

// TaxiService.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"

CRITICAL_SECTION cs;

driver* theFirst = NULL;

DWORD WINAPI CheckDrivers(LPVOID lpParam) 
{
	EnterCriticalSection(&cs);
		char *message = GetMessageFromClient();
		int driversID;
		if (message[1] == 'e')
			driversID = message[0] - 48;
		else
			driversID = (message[1] - 48) * 10 + message[0] - 48;
		
		location driversLocation = GetLocation(message);
		printf("Menjam lokaciju vozaca sa ID=%d na %d, %d.", driversID, driversLocation.x, driversLocation.y);
		theFirst = UpdateDriversLocation(driversID, driversLocation);

	LeaveCriticalSection(&cs);

	return 0;
}


void ShowDrivers(driver *head)
{
	driver *temp = head;

	if (temp == NULL)
		printf("Nema vozaca");
	else 
	{
		do 
		{
			printf("ID: %d\n", temp->ID);
			if (temp->available)
				printf("slobodan \n");
			else
				printf("zauzet \n");
			printf("na lokaciji: (%d,%d)\n", temp->loc.x, temp->loc.y );
			temp = temp->next;

			printf("\n");
		} while (temp != NULL);
	}
	
}


int main()
{
	/*char *clientsLocation = GetMessageFromClient();
	printf("Dobio sam adresu klijenta: %s\n", clientsLocation);
	location clLoc = GetLocation(clientsLocation);
	printf("...sto je lokacija: %d  , %d \n", clLoc.x, clLoc.y);

	getchar(); */

	driver* firstDriver = NULL;

	firstDriver = AllDrivers();
	ShowDrivers(firstDriver);


	DWORD checkD;
	HANDLE hCheckDrivers;

	InitializeCriticalSection(&cs);

	hCheckDrivers = CreateThread(NULL, 0, &CheckDrivers, NULL, 0, &checkD);
	
	getchar();
	ShowDrivers(theFirst);
	
	CloseHandle(hCheckDrivers);
	DeleteCriticalSection(&cs); 



	return 0;
}





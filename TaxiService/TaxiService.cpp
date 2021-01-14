// TaxiService.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCP.h"
#include "Functions.h"



DWORD WINAPI CheckClientAndDrivers(LPVOID lpParam)
{
	ServiceToClients();

	return 0;
}



int main()
{

	driver* firstDriver = NULL;
	firstDriver = AllDrivers();

	printf("Pocetak smene, vozaci se prijavljuju na stajalistu.\n\n");
	ShowDrivers(firstDriver);
	

	DWORD check;
	HANDLE hCheck;	

	hCheck = CreateThread(NULL, 0, &CheckClientAndDrivers, NULL, 0, &check);
	CloseHandle(hCheck);


	getchar(); 


	return 0;
}




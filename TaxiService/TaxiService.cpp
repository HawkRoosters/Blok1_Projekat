// TaxiService.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"





void ServiceToDrivers(driver** retDriver, location** loc)
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET driversSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	int iResult;
	char buffer[256];


	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());



	addrinfo *serverAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, "27016", &hints, &serverAddress);
	if (iResult != 0)
	{
		printf("get addrinfo failed, error: %d\n", iResult);
		WSACleanup();
	} 

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %ld\n", WSAGetLastError());
		freeaddrinfo(serverAddress);
		WSACleanup();
	}

	iResult = bind(listenSocket, serverAddress->ai_addr, (int)serverAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(serverAddress);
		closesocket(listenSocket);
		WSACleanup();
	}

	freeaddrinfo(serverAddress);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
	}


	printf("Taxi. Izvolite? \n");


	clientSocket = accept(listenSocket, NULL, NULL);
	if(clientSocket == INVALID_SOCKET)
	{
			if (WSAGetLastError() == WSAECONNRESET)
				printf("accept failed, because timeout for client request has expired.\n");
			else
				printf("accept failed with error: %d\n", WSAGetLastError());
	}


	iResult = recv(clientSocket, buffer, 256, 0);
	if (iResult > 0)
	{
		printf("Klijent se javlja sa: %s\n", buffer);
	}
	else if (iResult == 0)
	{
		printf("Razgovor s klijentom zavrsen.\n");
		closesocket(clientSocket);
	}
	else
	{
		printf("recv failed, error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
	}

	*(*loc) = GetLocation(buffer);

	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed, error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
	}



	printf("\nTaxi service trazi trenutne lokacije vozaca... \n");


	driver *theDriver;
	driver *ret = AllDrivers();


	driversSocket = accept(listenSocket, NULL, NULL);
	if (driversSocket == INVALID_SOCKET)
	{
		if (WSAGetLastError() == WSAECONNRESET)
			printf("accept failed, because timeout for client request has expired.\n");
		else
			printf("accept failed with error: %d\n", WSAGetLastError());
	}

	while (true)
	{
		iResult = recv(driversSocket, buffer, 256, 0);
		if (iResult > 0)
		{
			buffer[iResult] = '\0';
			theDriver = (driver *)buffer;
			printf("Vozac %d se nalazi na lokaciji:  %d, %d \n", theDriver->ID, theDriver->loc.x, theDriver->loc.y);

			ret = UpdateDriversLocation(ret, theDriver->ID, theDriver->loc);
			*(*retDriver) = *ret;
		}
		else if (iResult == 0)
		{
			closesocket(driversSocket);
			printf("\nTaxi service je proverio vozace. \n");
			break;
		}
		else
		{
			printf("recv failed, error: %d\n", WSAGetLastError());
			closesocket(driversSocket);
		}
	}


	closesocket(listenSocket);
	WSACleanup();
}





//CRITICAL_SECTION cs;

DWORD WINAPI CheckDrivers(LPVOID lpParam) 
{
	location l;
	l.x = 0;
	l.y = 0;
	location *ll;
	ll = &l;

	driver* first = *(driver**)lpParam;
		ServiceToDrivers(&first, &ll);
	printf("\n\nTrenutne lokacije vozaca: \n");

	return 0;
}


void ShowDrivers(driver *first)
{
	driver *temp = first;

	if (temp == NULL)
		printf("Nema vozaca.\n");
	else 
	{
		do 
		{
			printf("ID: %d\n", temp->ID );
			if ( temp->available)
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

	driver* firstDriver = NULL;
	firstDriver = AllDrivers();

	printf("Pocetak smene, vozaci se prijavljuju na stajalistu.\n\n");
	ShowDrivers(firstDriver);
	getchar();
	


	location l;
	l.x = 0;
	l.y = 0;	 
	location *ll;
	ll = &l;


	ServiceToDrivers(&firstDriver, &ll);
	getchar();

	ShowDrivers(firstDriver);
	getchar();


	printf("Lokacija kojoj trazim najblizu: (%d,%d)\n", (*ll).x, (*ll).y);
	getchar();

	driver* theDriver = (driver*)malloc(sizeof(driver));
	int *min = (int*)malloc(sizeof(int));
	theDriver = Find(firstDriver, *ll, &min);
	getchar();



	if (theDriver == NULL)
		printf("Trenutno nema slobodnih vozaca. Molimo, pozovite kasnije. \n\n");
	else if (*min == 0)
		printf("Poslat Vam je vozac %d, stize za minut.\n\n", theDriver->ID);
	else
		printf("Poslat Vam je vozac %d, stize za %d minuta.\n\n", theDriver->ID, *min ); 




	ShowDrivers(firstDriver);
	getchar();

	return 0;
}




/*DWORD checkD;
HANDLE hCheckDrivers;

hCheckDrivers = CreateThread(NULL, 0, &CheckDrivers, &firstDriver, 0, &checkD);
CloseHandle(hCheckDrivers);
getchar(); */
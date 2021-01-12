#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN


#include <iostream>

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include <windows.h>
#include <winSock2.h>
#include <ws2tcpip.h>

#include <time.h>

#include <cstring>
#include <string.h>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


typedef struct l
{
	int x;
	int y;
}location;


typedef struct taxidriver
{
	int ID;
	location loc;
	bool available;
	struct taxidriver* next;
} driver;


char* RandomLocation()
{
	char *ret = (char*)malloc(9 * sizeof(char));

	srand((unsigned int)time(NULL));
	int x = rand() % 100;
	int y = rand() % 100;

	ret[0] = '(';
	if (x > 9)
	{
		ret[1] = x / 10 + 48;
		ret[2] = x % 10 + 48;
		ret[3] = ',';
		if (y > 9)
		{
			ret[4] = y / 10 + 48;
			ret[5] = y % 10 + 48;
			ret[6] = ')';
			ret[7] = '\0';
		}
		else
		{
			ret[4] = y + 48;
			ret[5] = ')';
			ret[6] = '\0';
		}
	}
	else
	{
		ret[1] = x + 48;
		ret[2] = ',';
		if (y > 9)
		{
			ret[3] = y / 10 + 48;
			ret[4] = y % 10 + 48;
			ret[5] = ')';
			ret[6] = '\0';
		}
		else
		{
			ret[3] = y + 48;
			ret[4] = ')';
			ret[5] = '\0';
		}
	}

	return ret;
}



location GetLocation(char message[])
{
	location l;
	l.x = 0;
	l.y = 0;

	for (int i = 0; i < (int)strlen(message) + 1; i++)
	{
		if (message[i] == '(')
		{
			l.x = message[i + 1] - 48;
			if (message[i + 2] != ',')
				l.x = l.x * 10 + message[i + 2] - 48;
		}

		if (message[i] == ',')
		{
			l.y = message[i + 1] - 48;
			if (message[i + 2] != ')')
				l.y = l.y * 10 + message[i + 2] - 48;
		}
	}

	return l;
}


driver* AllDrivers()
{
	driver* first = NULL;
	driver* newDriver = NULL;
	driver* p = NULL;

	for (int i = 1; i < 6; i++)
	{
		newDriver = (driver*)malloc(sizeof(driver));
		newDriver->ID = i;

		char stajaliste[] = "(50,50)";
		newDriver->loc = GetLocation(stajaliste);
		newDriver->available = true;
		newDriver->next = NULL;

		if (first == NULL)
			first = newDriver;
		else
		{
			p = first;
			while (p->next != NULL)
				p = p->next;

			p->next = newDriver;
		}
	}

	return first;
}


driver* UpdateDriversLocation(driver* first,  int id, location newLoc)
{
	driver* temp = first;

	do {
		if (temp->ID == id) 
		{
			printf("Menjam lokaciju vozaca %d sa (%d, %d) na novu (%d, %d).\n", id, temp->loc.x, temp->loc.y, newLoc.x, newLoc.y);
			temp->loc = newLoc;
		}

		temp = temp->next;
	} while (temp != NULL);

	return first;
}




bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed %d\n", WSAGetLastError());
		return false;
	}

	return true;
}



void Answer(char* serviceToClient)
{
	SOCKET connectSocket = INVALID_SOCKET;
	int iResult;


	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());


	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %d\n", WSAGetLastError());
		WSACleanup();
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if ( serviceToClient )
		serverAddress.sin_port = htons(27018);
	else
		serverAddress.sin_port = htons(27017);

	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server. \n");
		closesocket(connectSocket);
		WSACleanup();
	}

	

	if (serviceToClient != NULL) 
	{
		char *message = serviceToClient;

		iResult = send(connectSocket, message, (int)strlen(message) + 1, 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed, error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
		}
	}
	else 
	{
		driver *theDriver = AllDrivers();

		while (theDriver != NULL)
		{
			theDriver->loc = GetLocation(RandomLocation());

			iResult = send(connectSocket, (char*)theDriver, (int)sizeof(driver), 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed, error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
			}


			printf("Vozac sa ID = %d prijavljuje svoju novu lokaciju: (%d,%d) \n", theDriver->ID, theDriver->loc.x, theDriver->loc.y);

			theDriver = theDriver->next;
			Sleep(1000);
		}
	}



	iResult = shutdown(connectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed, error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}


	Sleep(1000);
	WSACleanup();
}




void ClientsMessage(int ID)
{
	SOCKET connectSocket = INVALID_SOCKET;
	int iResult;

	if (!InitializeWindowsSockets())
		return;

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(27016);

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server. \n");
		closesocket(connectSocket);
		WSACleanup();
	}



		char *loc = RandomLocation();
		printf("Dobar dan. Moze li jedno vozilo na %s? \n", loc);
		iResult = send(connectSocket, loc, (int)strlen(loc) + 1, 0);
	


	if (iResult == SOCKET_ERROR)
	{
		printf("send failed, error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return;
	}

	closesocket(connectSocket);
	WSACleanup();
}




double destination(location driversL, location clientsL) 
{
	double absX = abs(driversL.x - clientsL.x);
	double absY = abs(driversL.y - clientsL.y);

	return sqrt( absX * absX + absY * absY );
}


void Find(driver *head, location cl, int** min, driver** ret) 
{
	*ret = NULL;
	double dest = destination(head->loc, cl);
	
	driver* temp = head;

	while (temp->next != NULL)
	{
		if (temp->available) 
		{
			if( destination(temp->loc, cl) <= dest )
				*ret = temp;
		}

		temp = temp->next;
	}


	if (ret == NULL)
		printf("Trenutno nema dostupnih vozaca. \n");
	else 
	{
		(*ret)->available = false;
		(*ret)->loc = cl;
		// 80km/h == 4/3 km/min ...  t = dest/(4/3)== 3dest/4 min
		//+ ~ 30s po km stajanje na semaforima 
		dest = dest / 10; //  realnije za rastojanja kroz grad, formulom se dobiju sati umsto min..
		*(*min) = (int)((3*dest/4) + 0.5*dest);
		printf("Najblizi je vozac %d, na %.2f km, treba mu %.0f minuta \n", (*ret)->ID, dest, (float)(*(*min)) );
	}

}


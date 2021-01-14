#pragma once
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

#include "Functions.h"

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
	if (serviceToClient)
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

		printf("\n\n");
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

	printf("\n %d. klijent: \n", ID);
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


	iResult = shutdown(connectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	closesocket(connectSocket);
	WSACleanup();
}
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
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup faild %d\n", WSAGetLastError());
		return false;
	}

	return true;
}





char* GetMessageFromClient()
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char recvbuf[7];

	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());

	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, "27016", &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("get addrinfo failed, error: %d\n", iResult);
		WSACleanup();
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
	}

	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
	}

	freeaddrinfo(resultingAddress);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
	}

	printf("Server initialized, waiting for clients.\n");

	acceptedSocket = accept(listenSocket, NULL, NULL);
	if (acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
	}


	iResult = recv(acceptedSocket, recvbuf, 7, 0);

	char *ret = (char*)malloc(7 * sizeof(char));
	for (int i = 0; i < 7; i++) 
	{
		ret[i] = recvbuf[i];

		if (recvbuf[i] == '\0')
			break;
	}

	if (iResult > 0) 
	{			
		printf("Message received from client: %s\n", ret);
	}
	else if (iResult == 0)
	{
		printf("Connection with client closed.\n");
		closesocket(acceptedSocket);
	}
	else
	{
		printf("recv failed, error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
	}



	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed, error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
	}


	return ret;
}


char* RandomLocation()
{
	char *ret = (char*)malloc(7 * sizeof(char));
	
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


void ClientRequest(int dID)
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

	char driversID[10];
	if ((dID > 0) && (dID < 10))
	{
		driversID[0] = dID + 48;
		driversID[1] = 'e';
	}
	else
	{
		if (dID >= 10) 
		{
			driversID[0] = dID / 10 + 48;
			driversID[1] = dID % 10 + 48;
		}
	}

	for (int i = 2; i < 10; i++)
		driversID[i] = RandomLocation()[i-2];

	if (dID == 0)
		iResult = send(connectSocket, RandomLocation(), (int)strlen(RandomLocation()) + 1, 0);
	else
		iResult = send(connectSocket, driversID, (int)strlen(driversID) + 1, 0 );


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
	driver* head = NULL;
	driver* newDriver = NULL;
	driver* p = NULL;

	for (int i = 1; i < 5; i++)
	{
		newDriver = (driver*)malloc(sizeof(driver));
		newDriver->ID = i;

		char stajaliste[] = "(50,50)";
		newDriver->loc = GetLocation(stajaliste);
		newDriver->available = true;
		newDriver->next = NULL;

		if (head == NULL)
			head = newDriver;
		else
		{
			p = head;
			while (p->next != NULL)
				p = p->next;

			p->next = newDriver;
		}
	}

	return head;
}


driver* UpdateDriversLocation(int id, location newLoc) 
{
	driver* head = AllDrivers();
	driver* temp = head;
	int i = 1;

	do {
		if (i == id) 
			temp->loc = newLoc;

		temp = temp->next;
		i++;
	} while (temp != NULL);

	return head;
}




float destination(location driversL, location clientsL) 
{
	return sqrt(abs(driversL.x - clientsL.x) + abs(driversL.y + clientsL.y));
}


driver* Find(driver *head, location cl) 
{
	driver* ret = NULL;
	float dest = destination(head->loc, cl);

	driver* temp = head;

	while (temp->next != NULL)
	{
		if (temp->available) 
		{
			if( destination(temp->loc, cl) < dest )
				ret = temp;
		}

		temp = temp->next;
	}

	if (ret == NULL)
		printf("Trenutno nema dostupnih vozača.");
	else 
	{
		ret->available = false;
		printf("Poslat Vam je vozac %d\n", ret->ID);
	}

	return ret;
}
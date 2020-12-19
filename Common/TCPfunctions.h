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
	char recvbuf[512];

	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());
	//return;

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
		//return;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		//return;
	}

	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
	//	return;
	}

	freeaddrinfo(resultingAddress);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		//return;
	}

	printf("Server initialized, waiting for clients.\n");

	acceptedSocket = accept(listenSocket, NULL, NULL);
	if (acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		//return;
	}

	iResult = recv(acceptedSocket, recvbuf, 512, 0);

	if (iResult > 0) 
	{			
		printf("Message received from client: %s\n", recvbuf);
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
	//	return;
	}

	char *ret = (char*)malloc(512 * sizeof(char));
	for (int i = 0; i < 512; i++)
		ret[i] = recvbuf[i];

	return ret;
}




void ClientRequest(char input[])
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

	iResult = send(connectSocket, input, (int)strlen(input) + 1, 0);
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


driver* Add(driver *head,  int x, int y) 
{
	driver *newDriver = NULL;
	driver *temp = head;
	int d = 0;

	newDriver = (driver*)malloc(sizeof(driver));
	newDriver->loc.x = x;
	newDriver->loc.y = y;
	newDriver->available = true;
	newDriver->next = NULL;

	while (temp->next != NULL) 
	{
		temp = temp->next;
		d++;
	}

	newDriver->ID = d;
	temp->next = newDriver;

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
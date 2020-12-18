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





void GetingClientRequest()
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char recvbuf[512];

	if (!InitializeWindowsSockets())
		return;

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
		return;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return;
	}

	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(resultingAddress);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	printf("Server initialized, waiting for clients.\n");

	acceptedSocket = accept(listenSocket, NULL, NULL);
	if (acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return;
	}


	iResult = recv(acceptedSocket, recvbuf, 512, 0);

	if (iResult > 0)
		printf("Message received from client: %s\n", recvbuf);
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
		return;
	}


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



/*void ResponseToClient()
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
		printf("Unable to connect . \n");
		closesocket(connectSocket);
		WSACleanup();
	}

	char input[] = "Service's response to client...\n";
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


} */
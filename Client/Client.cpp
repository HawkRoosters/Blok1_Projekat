// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.



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



int main()
{
    char messageToSend[] = "te pido que todos los viernes sean una fiesta";
	ClientRequest(messageToSend);



	getchar();
	return 0;
}







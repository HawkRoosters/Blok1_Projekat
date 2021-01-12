// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"


void AnswerFromService()
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	int iResult;
	char buffer[256];

	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());

	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, "27018", &hints, &resultingAddress);
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


	printf("\nCekam odgovor Taxi service-a...  \n");

	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept failed, error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
	}


	iResult = recv(clientSocket, buffer, 256, 0);
	if (iResult > 0)
	{
		printf("Dobio sam odgovor od Taxi service-a: %s\n", buffer);
	}
	else if (iResult == 0)
	{
		printf("Razgovor sa Taxi service-om zavrsen.\n");
		closesocket(clientSocket);
	}
	else
	{
		printf("recv failed, error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
	}

	
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed, error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
	}



}


DWORD WINAPI ClientToService(LPVOID lpParam)
{
	int id = *(int*)lpParam;
	printf("\n %d. klijent: \n", id);

	ClientsMessage(id);
	AnswerFromService();


	return 0;
}

int main()
{

	DWORD request1, request2;
	HANDLE hRequest, hRequest2;

	int id1 = 1;
	int id2 = 2;

	hRequest = CreateThread(NULL, 0, &ClientToService, &id1, 0, &request1);
	//Sleep(60000);
	//hRequest2 = CreateThread(NULL, 0, &ClientToService, &id2, 0, &request2);

	CloseHandle(hRequest);
//	CloseHandle(hRequest2);

	//Sleep(5000);
	getchar();


	return 0;
}







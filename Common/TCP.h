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





void ServiceToDrivers(driver** retDriver)
{
	SOCKET listenDriversSocket = INVALID_SOCKET;
	SOCKET driversSocket = INVALID_SOCKET;

	char buffer[256];
	int iResult;

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress.sin_port = htons(27017);


	listenDriversSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenDriversSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %ld\n", WSAGetLastError());
		WSACleanup();
	}

	iResult = bind(listenDriversSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		closesocket(listenDriversSocket);
		WSACleanup();
	}

	iResult = listen(listenDriversSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenDriversSocket);
		WSACleanup();
	}


	driversSocket = accept(listenDriversSocket, NULL, NULL);
	if (driversSocket == INVALID_SOCKET)
	{
		if (WSAGetLastError() == WSAECONNRESET)
			printf("accept failed, because timeout for client request has expired.\n");
		else
			printf("accept failed with error: %d\n", WSAGetLastError());
	}


	driver *theDriver;
	driver *ret = AllDrivers();

	while (true)
	{
		iResult = recv(driversSocket, buffer, 256, 0);
		if (iResult > 0)
		{
			theDriver = (driver *)buffer;
			printf("Vozac %d se nalazi na lokaciji:  %d, %d \n", theDriver->ID, theDriver->loc.x, theDriver->loc.y);

			ret = UpdateDriversLocation(ret, theDriver->ID, theDriver->loc);
			*(*retDriver) = *ret;
		}
		else
		{
			if (iResult == 0)
				printf("\nTaxi service je proverio vozace. \n");
			else
				printf("recv failed, error: %d\n", WSAGetLastError());


			closesocket(driversSocket);
			break;
		}
	}


	closesocket(listenDriversSocket);
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



void ServiceToClients()
{
	SOCKET listenClientsSocket = INVALID_SOCKET;
	SOCKET clientSockets[4];
	int index = 0;

	int iResult;
	char buffer[256];

	if (!InitializeWindowsSockets())
		printf("Initialize socket, failed.", WSAGetLastError());


	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress.sin_port = htons(27016);

	memset(clientSockets, 0, 4 * sizeof(SOCKET));

	listenClientsSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenClientsSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %ld\n", WSAGetLastError());
		WSACleanup();
	}


	iResult = bind(listenClientsSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed, error: %d\n", WSAGetLastError());
		closesocket(listenClientsSocket);
		WSACleanup();
	}


	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenClientsSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char *)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	unsigned long  nonBlockingMode = 1;
	if (ioctlsocket(listenClientsSocket, FIONBIO, &nonBlockingMode) != 0)
		printf("ioctlsocket failed with error.");


	iResult = listen(listenClientsSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenClientsSocket);
		WSACleanup();
	}


	printf("Taxi. Izvolite? \n\n");


	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	location loc;

	while (true)
	{
		FD_ZERO(&set);

		if (index != 4)
			FD_SET(listenClientsSocket, &set);

		for (int i = 0; i < index; i++)
			FD_SET(clientSockets[i], &set);

		int sResult = select(0, &set, NULL, NULL, &timeVal);
		if (sResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenClientsSocket);
			WSACleanup();
		}
		else if (sResult == 0)
			continue;
		else if (FD_ISSET(listenClientsSocket, &set))
		{
			sockaddr_in clientAddress;
			int clientAddressSize = sizeof(struct sockaddr_in);

			clientSockets[index] = accept(listenClientsSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
			if (clientSockets[index] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
					printf("accept failed, because timeout for client request has expired.\n");
				else
					printf("accept failed, error: %d\n", WSAGetLastError());
			}
			else
			{
				if (ioctlsocket(clientSockets[index], FIONBIO, &nonBlockingMode))
				{
					printf("ioctlsocket failed, error");
					continue;
				}

				index++;
				printf("\nUspostavljena veza sa %d. kljentom. \n", index);
				Sleep(2000);
			}
		}
		else
		{
			for (int i = 0; i < index; i++)
			{
				if (FD_ISSET(clientSockets[i], &set))
				{
					iResult = recv(clientSockets[i], buffer, 256, 0);
					if (iResult > 0)
					{
						buffer[iResult] = '\0';
						printf("\nNovi klijent se javlja sa: %s\n", buffer);
						printf("\nTaxi service trazi trenutne lokacije vozaca... \n");
						Sleep(2000);

						driver* theFirst = (driver*)malloc(sizeof(driver));
						driver* theDriver = (driver*)malloc(sizeof(driver));
						int *min = (int*)malloc(sizeof(int));

						ServiceToDrivers(&theFirst);

						printf("\nTrenutne lokacije vozaca: \n");
						ShowDrivers(theFirst);
						Sleep(2000);

						loc = GetLocation(buffer);
						printf("Lokacija kojoj trazim najblizu, : (%d,%d)\n", loc.x, loc.y);
						Find(theFirst, loc, &min, &theDriver);


						if (theDriver == NULL)
						{
							char answer[60] = "Trenutno nema slobodnih vozaca. Molimo, pozovite kasnije.";
							Answer(answer);
						}
						else
						{
							char answer[55] = "Poslat Vam je vozac ";
							answer[20] = theDriver->ID + 48;

							char temp[12] = ", stize za ";
							for (int i = 0; i < 11; i++)
								answer[i + 21] = temp[i];

							if (*min == 0)
							{
								char tmp[] = " minut.";
								for (int i = 0; i < 7; i++)
									answer[i + 33] = tmp[i];
							}
							else
							{
								answer[32] = *min / 10 + 48;
								answer[33] = *min % 10 + 48;

								char tmp[] = " minuta.";
								for (int i = 0; i < 8; i++)
									answer[i + 34] = tmp[i];
							}

							Answer(answer);
						}

						Sleep(2000);
						printf("\nTrenutne lokacije vozaca, nakon slanja poruke klijentu: \n");
						ShowDrivers(theFirst);
					}
					else
					{
						closesocket(clientSockets[i]);

						for (int j = i; j < index - 1; j++)
							clientSockets[j] = clientSockets[j + 1];

						clientSockets[index - 1] = 0;
						index--;
					}

				}

			}
		}
	}

	closesocket(listenClientsSocket);
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


	closesocket(listenSocket);
}

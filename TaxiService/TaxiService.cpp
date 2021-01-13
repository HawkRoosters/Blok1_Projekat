// TaxiService.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCPfunctions.h"

void ShowDrivers(driver *first)
{
	driver *temp = first;

	if (temp == NULL)
		printf("Nema vozaca.\n");
	else
	{
		do
		{
			printf("ID: %d\n", temp->ID);
			if (temp->available)
				printf("slobodan \n");
			else
				printf("zauzet \n");
			printf("na lokaciji: (%d,%d)\n", temp->loc.x, temp->loc.y);
			temp = temp->next;

			printf("\n");
		} while (temp != NULL);
	}
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
			if(iResult == 0)
				printf("\nTaxi service je proverio vozace. \n");
			else 
				printf("recv failed, error: %d\n", WSAGetLastError());
			
			
			closesocket(driversSocket);
			break;
		}
	}


	closesocket(listenDriversSocket);
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
				printf("Uspostavljena veza sa %d. kljentom. \n", index);
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
						printf("\nNovi klijent se javlja sa: %s\n",  buffer); 						

						printf("\nTaxi service trazi trenutne lokacije vozaca... \n");


						driver* theFirst = (driver*)malloc(sizeof(driver));
						driver* theDriver = (driver*)malloc(sizeof(driver));
						int *min = (int*)malloc(sizeof(int));

						ServiceToDrivers(&theFirst);

						printf("\nTrenutne lokacije vozaca: \n");
						ShowDrivers(theFirst);

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


						printf("\nTrenutne lokacije vozaca, nakon slanja poruke klijentu: \n");
						ShowDrivers(theFirst);
					}
					else
					{
						if (iResult == 0)
							printf("\nRazgovor s klijentom zavrsen.\n");
						else
							printf("recv failed, error: %d\n", WSAGetLastError());


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




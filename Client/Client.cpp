// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "TCP.h"
#include "Functions.h"



HANDLE hSemaphores[2];

DWORD WINAPI ClientToService(LPVOID lpParam)
{
		if ((int)lpParam == 0) 
		{
			WaitForSingleObject(hSemaphores[0], INFINITE);

			ClientsMessage((int)lpParam + 1);
			AnswerFromService();

			ReleaseSemaphore(hSemaphores[1], 1, NULL);
			CloseHandle(hSemaphores[0]);
		}
		else 
		{
			WaitForSingleObject(hSemaphores[1], INFINITE);

			ClientsMessage((int)lpParam + 1);
			AnswerFromService();

			CloseHandle(hSemaphores[1]);		
		}


	return 0;
}

int main()
{

	DWORD request1, request2;
	HANDLE hRequest1, hRequest2;

	hRequest1 = NULL;
	hRequest2 = NULL;	

	hSemaphores[0] = CreateSemaphore(0, 1, 1, NULL);
	hSemaphores[1] = CreateSemaphore(0, 0, 1, NULL);


	if (hSemaphores[0]) 
	{
		hRequest1 = CreateThread(NULL, 0, &ClientToService, (LPVOID)0, 0, &request1);
	}

	if (hSemaphores[1]) 
	{
		hRequest2 = CreateThread(NULL, 0, &ClientToService, (LPVOID)1, 0, &request2);
		SetSemaphore();
	}



	if (hRequest1)
		CloseHandle(hRequest1);

	if (hRequest2)
		CloseHandle(hRequest2);



	getchar();

	return 0;
}







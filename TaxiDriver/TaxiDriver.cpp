// TaxiDriver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include "TCP.h"
#include "Functions.h"


DWORD WINAPI DriversAnswer(LPVOID lpParam)
{
	Answer(NULL);

	return 0;
}



int main()
{
	DWORD answer1, answer2;
	HANDLE hAnswer1, hAnswer2;
	bool s = false;

	hAnswer1 = CreateThread(NULL, 0, &DriversAnswer, NULL, 0, &answer1);
	CloseHandle(hAnswer1);

	Sleep(15000);

	while (true)
	{
		if (GetSemaphore())
		{
			hAnswer2 = CreateThread(NULL, 0, &DriversAnswer, NULL, 0, &answer2);
			CloseHandle(hAnswer2);
			break;
		}
	}



	getchar();


	return 0;
}

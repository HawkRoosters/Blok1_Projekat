// TaxiDriver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "TCPfunctions.h"


DWORD WINAPI DriversAnswer(LPVOID lpParam)
{
	Answer(NULL);

	return 0;
}



int main()
{
	DWORD answer1, answer2;
	HANDLE hAnswer1, hAnswer2;


	hAnswer1 = CreateThread(NULL, 0, &DriversAnswer, NULL, 0, &answer1);
	CloseHandle(hAnswer1);

	//getchar();
	Sleep(12000);

	hAnswer2 = CreateThread(NULL, 0, &DriversAnswer, NULL, 0, &answer2);
	CloseHandle(hAnswer2);

	getchar();



	return 0;
}

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
	DWORD answer;
	HANDLE hAnswer;


	hAnswer = CreateThread(NULL, 0, &DriversAnswer, NULL, 0, &answer);
	CloseHandle(hAnswer);

	Sleep(5000);

	return 0;
}

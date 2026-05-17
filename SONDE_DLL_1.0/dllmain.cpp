// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"
#include <windows.h>
#include <cstdio>
#include <chrono>
#include <fstream>
#pragma warning(disable : 4996)
using namespace std;
using namespace std::chrono;
extern ofstream Test;
extern const char* Test_Name;
time_t start_time, stop_time;
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// Будьте внимательны!!! Функция printf используется исключительно для примера.
		// Библиотека будет работать только в консольном приложении.
		// В обычное приложение под Windows подключать эту библиотеку нельзя!
		//printf("Hello, I am SONDE library!\n");
		// чистим файл     
		Test.open(Test_Name, ios::out | ios::trunc);
		start_time = system_clock::to_time_t(system_clock::now());
		Test << "lib is open "  << ctime(&start_time) << endl;
		
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		
		stop_time = system_clock::to_time_t(system_clock::now());
		Test << "lib is close " << ctime(&stop_time) << endl;
		Test.close();
		break;
	}
	return TRUE;
}


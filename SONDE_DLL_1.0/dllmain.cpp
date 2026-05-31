// dllmain.cpp : точка входа DLL.
// Управление жизненным циклом диагностического лога делегировано модулю Logger.

#include "stdafx.h"
#include <windows.h>
#include "Logger.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		logger::init();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		logger::shutdown();
		break;
	}
	return TRUE;
}

#include "stdafx.h"
#include "ErrorState.h"

namespace {
thread_local std::string g_lastError;
}

void ClearSondeLastError() {
	g_lastError.clear();
}

void SetSondeLastError(const std::string& message) {
	g_lastError = message;
}

const char* GetSondeLastError() {
	return g_lastError.c_str();
}

extern "C" __declspec(dllexport) const char* sonde_get_last_error() {
	return GetSondeLastError();
}

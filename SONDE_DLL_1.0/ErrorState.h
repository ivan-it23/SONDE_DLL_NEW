#pragma once

#include <string>

// Подробность последней ошибки хранится отдельно для каждого вызывающего потока.
// Числовой код остаётся основным ABI-контрактом, а строка объясняет конкретное поле
// метрологии, путь к весам или неверный аргумент.
void ClearSondeLastError();
void SetSondeLastError(const std::string& message);
const char* GetSondeLastError();

extern "C" __declspec(dllexport) const char* sonde_get_last_error();

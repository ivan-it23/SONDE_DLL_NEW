#pragma once
// Logger.h
// Единая точка управления диагностическим логом DLL (файл Test.txt) и флагом
// отладки. Глобальные имена Test и debug сохранены для совместимости с кодом,
// который обращается к ним через extern.

#include <fstream>

extern std::ofstream Test;     // поток диагностического лога
extern bool debug;             // признак включённой отладки (debug_mode)
extern const char* Test_Name;  // имя файла лога

namespace logger {

// Открывает лог-файл и фиксирует время старта. Вызывается при загрузке DLL.
void init();

// Фиксирует время завершения и закрывает лог-файл. Вызывается при выгрузке DLL.
void shutdown();

} // namespace logger

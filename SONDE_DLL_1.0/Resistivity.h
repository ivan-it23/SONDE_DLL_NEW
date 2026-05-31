#pragma once
// Resistivity.h
// Преобразования "фаза <-> УЭС" для бесконечной однородной среды и компенсация
// влияния скважины. Низкоуровневые функции используются модулями расчёта,
// экспортируемые функции коррекции определены в Resistivity.cpp.

#include "Types.h"

// Фаза от УЭС для бесконечной однородной среды.
float dFI(SONDE_PARAM param, float Ro);

// УЭС от фазы методом золотого сечения.
float RO_dFI(SONDE_PARAM param, double dfi);

// Фазовая поправка за влияние скважины.
float DFI_bhole(SONDE_PARAM param, float D_bh_mm, float ro_bh);

#pragma once
// Resistivity.h
// Преобразования "сигнал зонда <-> УЭS" для бесконечной однородной среды по
// фазовому и амплитудному (затухание в дБ) каналам. Низкоуровневые функции
// используются модулями расчёта, экспортируемые функции коррекции определены
// в Resistivity.cpp.

#include <complex>
#include "Types.h"

// Комплексный сигнал зонда (отношение приёмников) для однородной среды.
// arg(SIGNAL) — фазовый сдвиг (рад), abs(SIGNAL) — амплитудное отношение (разы).
std::complex<float> SIGNAL(SONDE_PARAM param, float ro);

// УЭС от фазового сдвига методом золотого сечения (arg(SIGNAL)).
float RO_ARG(SONDE_PARAM param, double dfi);

// УЭС от затухания в дБ методом золотого сечения (abs(SIGNAL)).
float RO_ATT(SONDE_PARAM param, double att_dB);

#pragma once
// Resistivity.h
// Преобразования "сигнал зонда <-> УЭС" для бесконечной однородной среды по
// фазовому и амплитудному (затухание в дБ) каналам.

#include <complex>
#include "Types.h"

// Комплексный сигнал зонда (отношение сигналов на приёмниках) для однородной среды.
// arg(SIGNAL) — фазовый сдвиг (рад), abs(SIGNAL) — амплитудное отношение (разы).
std::complex<float> SIGNAL(SONDE_PARAM param, float ro);

// УЭС от фазы по золотому сечению.
float RO_ARG(SONDE_PARAM param, double dfi);

// УЭС от затухания в дБ по золотому сечению.
float RO_ATT(SONDE_PARAM param, double att_dB);

// УЭС по фазе и по затуханию для всех зондов, без учёта скважины и зоны проникновения.
int compute_rho(CAL_SIGNAL* cal_signal, RHO* rho);

// По вычисленному и требуемому УЭС в опорной точке корректирует УЭС в искомой
// точке. Читает собственный файл метрологии и работает без sonde_set.
int correct_rho_to_ref_point(
	const char* metrologyPath,
	RHO* rho_calk_ref_point,
	RHO* rho_need_ref_point,
	RHO* rho_calk_desired_point,
	RHO* rho_required_desired_point);

// Восстанавливает из УЭС симметризованные фазу и затухание в дБ.
int compute_signal_from_rho(RHO* rho_calk, CAL_SIGNAL* cal_signal);

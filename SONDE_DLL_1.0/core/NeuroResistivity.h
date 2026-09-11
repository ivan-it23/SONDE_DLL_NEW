#pragma once
// NeuroResistivity.h
// Расчёт истинного УЭС пласта и параметров зоны проникновения нейросетью.

#include "Types.h"

// Заполняет фазовые УЭС по зондам (золотое сечение) и параметры зоны
// проникновения rho_p / rho_zp / R_zp по предсказанию нейросети.
// В service возвращается разброс фазовых УЭС по активным зондам, %.
int compute_true_rho_neuro(CAL_SIGNAL* cal_signal, RHO* rho, SERVICE* service);

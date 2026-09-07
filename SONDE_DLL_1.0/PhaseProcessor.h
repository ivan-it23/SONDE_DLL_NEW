#pragma once
// PhaseProcessor.h
// Извлечение и калибровка фаз из кадра GP_DATA, симметризация фаз с учётом
// работоспособности передатчиков. Матрицы коэффициентов симметризации
// вычисляются formula_simmetry.

#include "Types.h"

// Коэффициенты симметризации для текущей схемы (3/4/5 передатчиков).
// condition — один байт работоспособности; N_Tx — число передатчиков.
void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx);

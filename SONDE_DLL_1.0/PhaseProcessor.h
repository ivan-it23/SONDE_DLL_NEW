#pragma once
// PhaseProcessor.h
// Извлечение и калибровка фаз из кадра GP_DATA, симметризация фаз с учётом
// работоспособности передатчиков. Матрицы коэффициентов симметризации
// вычисляются formula_simmetry/formula_simmetry_old.

#include "Types.h"

// Коэффициенты симметризации для 4-передатчиковой схемы (историческая версия).
uint8_t formula_simmetry_old(float K[4][4], uint8_t condition);

// Коэффициенты симметризации для текущей схемы (3/4/5 передатчиков).
// condition — один байт работоспособности; N_Tx — число передатчиков.
void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx);

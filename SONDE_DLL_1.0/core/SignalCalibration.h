#pragma once
// SignalCalibration.h
// Извлечение данных из кадра GP_DATA, калибровка фазового и амплитудного
// каналов на нули воздуха и симметризация сигнала матрицей коэффициентов.

#include "Types.h"

// Заполняет матрицу коэффициентов симметризации K[5][5].
// condition — один байт от GP_DATA.uint32_t condition; N_Tx — число передатчиков.
void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx);

// Возвращает из кадра готовые с контроллера симметризованные сигналы и УЭС по
// фазовому и амплитудному каналам. Амплитудный канал читается только из
// расширенной структуры данных.
int extract_express_data(void* data, int shift, CAL_SIGNAL* cal_signal, RHO* rho);

// Возвращает калиброванные на воздух фазы и амплитудные затухания в дБ.
int calibrate_signal(void* data, int shift, CAL_SIGNAL* cal_signal);

// Возвращает байт работоспособности передатчиков из кадра.
int extract_condition(void* data, int shift, uint32_t* condition);

// Симметризует фазовый и амплитудный каналы матрицей коэффициентов, выбранной
// по числу передатчиков и байту работоспособности.
int symmetrize_signal(CAL_SIGNAL* cal_signal_in, CAL_SIGNAL* cal_signal_smt, uint32_t condition);

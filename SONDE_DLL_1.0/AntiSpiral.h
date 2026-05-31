#pragma once
// AntiSpiral.h
// Подавление спиральной помехи: дискретное преобразование Фурье, решение СЛАУ
// методом Гаусса и гармоническая очистка сигнала.

#include <complex>

// Дискретное преобразование Фурье на окне win.
void DFT(double *SGN, std::complex<double> *Harm, int win);

// Решение системы линейных уравнений 5x5 методом Гаусса с выбором главного
// элемента. Возвращает 0 при успехе.
int SLAU(double matrica_a[5][5], int n, double massiv_b[5], double x[5]);

// Гармоническая очистка окна сигнала. Возвращает 0 при успехе.
int harmonics_clear(double *Sgn, double *Sgn_out, double *Sgn_out_m, int win);

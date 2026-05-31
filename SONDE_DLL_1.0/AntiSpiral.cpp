#include "stdafx.h"
#include "AntiSpiral.h"
#include "Constants.h"

#include <complex>
#include <cmath>

using namespace std;

void DFT(double *SGN, complex <double> *Harm, int win) {
	double win_2 = win / 2.0;
	for (int i = 0; i < win_2; i++)
		Harm[i] = 0;
	for (int k = 0; k < win_2; k++) {
		for (int n = 0; n < win; n++) {
			Harm[k] += SGN[n] * exp((std::complex<double>(0.0, -1.0) * PI * double(k*(n))) / win_2);
		}
		Harm[k] /= win_2;
	}
}

int SLAU(double matrica_a[5][5], int n, double massiv_b[5], double x[5]) {
	n = 5;
	int i, j, k, r;
	double c = 0, M = 0, max = 0, s = 0, a[5][5] = { 0.0f, }, b[5] = { 0.0f, };
	for (i = 0; i < n; i++) { x[i] = 0; }
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			a[i][j] = matrica_a[i][j];
		}
	}

	for (i = 0; i < n; i++) {
		b[i] = massiv_b[i];
	}

	for (k = 0; k < n; k++)
	{
		max = fabs(a[k][k]);
		r = k;
		for (i = k + 1; i < n; i++) {
			if (fabs(a[i][k]) > max) {
				max = fabs(a[i][k]);
				r = i;
			}
		}

		for (j = 0; j < n; j++) {
			c = a[k][j]; a[k][j] = a[r][j]; a[r][j] = c;
		}

		c = b[k]; b[k] = b[r]; b[r] = c;

		for (i = k + 1; i < n; i++) {
			for (M = a[i][k] / a[k][k], j = k; j < n; j++) {
				a[i][j] -= M * a[k][j];
			}
			b[i] -= M * b[k];
		}
	}

	if (a[n - 1][n - 1] == 0)

		if (b[n - 1] == 0) return -1;

		else return -2;

	else {
		for (i = n - 1; i >= 0; i--) {
			for (s = 0, j = i + 1; j < n; j++) {
				s += a[i][j] * x[j];
			}
			x[i] = (b[i] - s) / a[i][i];
		}
		return 0;
	}
}

int harmonics_clear(double *Sgn, double *Sgn_out, double *Sgn_out_m, int win) {
	complex <double> *HARM = new complex <double>[win];
	double min = 0;
	double max = 0;
	int n_max = 0;
	int n_remove = 0;
	bool stop = false;
	double win_2 = win / 2.0;

	DFT(Sgn, HARM, win);

	for (int i = 0; i < win_2; i++) {
		if (abs(HARM[i + 1]) <= abs(HARM[i + 2]) && abs(HARM[i + 1]) <= abs(HARM[i]) && stop == false) {
			n_remove = i + 1; stop = true;
		}
	}
	for (int i = n_remove; i < win_2; i++) {
		if (abs(HARM[i]) > max) {
			max = abs(HARM[i]);
			n_max = i;
		}
	}

	double T = win / (n_max - real((HARM[n_max + 1] - HARM[n_max - 1]) / (2.0 * HARM[n_max] - HARM[n_max - 1] - HARM[n_max + 1])));
	if (T <= 0) return 1;

	double B[5][5] = { 0.0f, }, D[5] = { 0.0f, }; double X[5] = { 0.0f, };
	double Scosfi = 0, Ssinfi = 0, Ssinficosfi = 0, SsinKfi = 0, ScosKfi = 0;
	double Scos2ficosfi = 0, Ssin2fisinfi = 0, Ssin2ficosfi = 0, Scos2fisinfi = 0;
	double Ssin2ficos2fi = 0, Ssin2fi = 0, Scos2fi = 0, SsinK2fi = 0, ScosK2fi = 0;
	double V_ = 0, V_cosfi = 0, V_sinfi = 0, V_cos2fi = 0, V_sin2fi = 0;
	double Angle = 0;

	for (int i = 0; i < win; i++) {
		Angle = double(i) * 2.0 * PI / T;
		B[0][1] += cos(Angle);
		B[0][2] += sin(Angle);
		B[1][2] += sin(Angle)*cos(Angle);
		B[1][1] += pow(cos(Angle), 2);
		B[2][2] += pow(sin(Angle), 2);

		B[1][3] += cos(2 * Angle)*cos(Angle);
		B[2][4] += sin(2 * Angle)*sin(Angle);
		B[1][4] += sin(2 * Angle)*cos(Angle);
		B[2][3] += cos(2 * Angle)*sin(Angle);
		B[3][4] += sin(2 * Angle)*cos(2 * Angle);
		B[0][4] += sin(2 * Angle);
		B[0][3] += cos(2 * Angle);
		B[4][4] += pow(sin(2 * Angle), 2);
		B[3][3] += pow(cos(2 * Angle), 2);

		D[0] += Sgn[i];
		D[1] += Sgn[i] * cos(Angle);
		D[2] += Sgn[i] * sin(Angle);
		D[3] += Sgn[i] * cos(2 * Angle);
		D[4] += Sgn[i] * sin(2 * Angle);
	}
	B[0][0] = win;
	B[1][0] = B[0][1]; B[2][0] = B[0][2]; B[3][0] = B[0][3]; B[4][0] = B[0][4];
	B[2][1] = B[1][2]; B[3][1] = B[1][3]; B[4][1] = B[1][4];
	B[3][2] = B[2][3]; B[4][2] = B[2][4]; B[4][3] = B[3][4];

	if (SLAU(B, 5, D, X) != 0) {
		return 1;
	}

	if (isnan(X[0]) || isnan(X[1]) || isnan(X[2]) || isnan(X[3]) || isnan(X[4]))return 1;

	for (int i = 0; i < win; i++) {
		double angle = double(i) * 2.0 * PI / T;
		Sgn_out[i] = X[1] * cos(angle) + X[2] * sin(angle) + X[3] * cos(2 * angle) + X[4] * sin(2 * angle);
	}

	*Sgn_out_m = Sgn[(int)(win_2)] - Sgn_out[(int)(win_2)];
	delete HARM;
	return 0;
}

extern "C" __declspec(dllexport) int anti_spiral(double *Sgn_in, double *Sgn_out, int length, int win_f, int win_ma) {
	if (win_ma < 1 || win_ma > win_f) win_ma = 1;
	double *Income_buff = new double[win_f];
	double *Output_buff = new double[win_f];
	double *Sgn_ma_buff = new double[win_ma];
	double Sgn_ma_summ = 0;
	double Sgn_out_buff = 0;
	double Out_m;
	int w_ma = 0;
	for (int i = 0; i < win_ma; i++) {
		Sgn_ma_buff[i] = 0;
	}
	for (int i = 0; i < win_f; i++) {
		Income_buff[i] = 0;
		Output_buff[i] = 0;
	}
	for (int n = 0; n < length - win_f; n++) {
		for (int i = 0; i < win_f; i++) {
			Income_buff[i] = Sgn_in[n + i];
		}
		if (n < win_f / 2) {
			Sgn_out[n] = 0;
		}

		if (win_f > 4) {
			if (!harmonics_clear(Income_buff, Output_buff, &Out_m, win_f))
				Sgn_out_buff = Out_m;
			else
				Sgn_out_buff = Sgn_in[n + win_f / 2];
		}
		else
			Sgn_out_buff = Sgn_in[n + win_f / 2];

		//скользящее среднее
		Sgn_ma_summ -= Sgn_ma_buff[w_ma];
		Sgn_ma_buff[w_ma] = Sgn_out_buff;
		Sgn_ma_summ += Sgn_ma_buff[w_ma];

		w_ma++;
		if (w_ma == win_ma)w_ma = 0;

		Sgn_out[n + win_f / 2 - win_ma / 2] = Sgn_ma_summ / (double)win_ma;
	}

	for (int n = length - win_f; n < length; n++) {
		Sgn_out[n + win_f / 2 - win_ma / 2] = 0;
	}

	delete Income_buff; delete Output_buff;
	return 0;
}

#include "stdafx.h"
#include "AntiSpiral.h"
#include "Constants.h"
#include "ErrorState.h"

#include <complex>
#include <cmath>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

void DFT(double *SGN, complex <double> *Harm, int win) {
	if (!SGN || !Harm || win <= 0)
		return;
	const int halfWindow = win / 2;
	const double win_2 = win / 2.0;
	for (int i = 0; i < halfWindow; i++)
		Harm[i] = 0;
	for (int k = 0; k < halfWindow; k++) {
		for (int n = 0; n < win; n++) {
			Harm[k] += SGN[n] * exp((std::complex<double>(0.0, -1.0) * PI * double(k*(n))) / win_2);
		}
		Harm[k] /= win_2;
	}
}

int SLAU(double matrica_a[5][5], int n, double massiv_b[5], double x[5]) {
	if (!matrica_a || !massiv_b || !x || n < 1 || n > 5)
		return -3;
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
		if (max <= (std::numeric_limits<double>::epsilon)())
			return -2;

		for (i = k + 1; i < n; i++) {
			for (M = a[i][k] / a[k][k], j = k; j < n; j++) {
				a[i][j] -= M * a[k][j];
			}
			b[i] -= M * b[k];
		}
	}

	if (fabs(a[n - 1][n - 1]) <= (std::numeric_limits<double>::epsilon)())

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
	if (!Sgn || !Sgn_out || !Sgn_out_m || win < 6)
		return err::kInvalidArgument;
	std::vector<complex<double>> harmonics(static_cast<size_t>(win));
	double maxAmplitude = 0;
	int n_max = 0;
	int n_remove = 1;
	bool stop = false;
	const int halfWindow = win / 2;
	const double win_2 = win / 2.0;

	DFT(Sgn, harmonics.data(), win);

	for (int i = 0; i + 2 < halfWindow; i++) {
		if (abs(harmonics[i + 1]) <= abs(harmonics[i + 2]) &&
			abs(harmonics[i + 1]) <= abs(harmonics[i]) && !stop) {
			n_remove = i + 1; stop = true;
		}
	}
	for (int i = (std::max)(n_remove, 1); i + 1 < halfWindow; i++) {
		if (abs(harmonics[i]) > maxAmplitude) {
			maxAmplitude = abs(harmonics[i]);
			n_max = i;
		}
	}
	if (n_max <= 0 || n_max + 1 >= halfWindow || maxAmplitude <= 0.0)
		return err::kNumericalFailure;

	const complex<double> interpolationDenominator =
		2.0 * harmonics[n_max] - harmonics[n_max - 1] - harmonics[n_max + 1];
	if (abs(interpolationDenominator) <= (std::numeric_limits<double>::epsilon)())
		return err::kNumericalFailure;
	const double correction = real(
		(harmonics[n_max + 1] - harmonics[n_max - 1]) / interpolationDenominator);
	const double periodDenominator = n_max - correction;
	if (!std::isfinite(periodDenominator) || periodDenominator <= 0.0)
		return err::kNumericalFailure;
	const double T = win / periodDenominator;
	if (!std::isfinite(T) || T <= 0.0)
		return err::kNumericalFailure;

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
		return err::kNumericalFailure;
	}

	for (int i = 0; i < 5; ++i)
		if (!std::isfinite(X[i])) return err::kNumericalFailure;

	for (int i = 0; i < win; i++) {
		double angle = double(i) * 2.0 * PI / T;
		Sgn_out[i] = X[1] * cos(angle) + X[2] * sin(angle) + X[3] * cos(2 * angle) + X[4] * sin(2 * angle);
	}

	*Sgn_out_m = Sgn[(int)(win_2)] - Sgn_out[(int)(win_2)];
	return err::kOk;
}

extern "C" __declspec(dllexport) int anti_spiral(double *Sgn_in, double *Sgn_out, int length, int win_f, int win_ma) {
	ClearSondeLastError();
	if (!Sgn_in || !Sgn_out) {
		SetSondeLastError("anti_spiral requires non-null input and output arrays.");
		return err::kInvalidArgument;
	}
	if (length <= 0 || win_f < 6 || win_f > length || win_ma < 1 || win_ma > win_f) {
		SetSondeLastError("anti_spiral requires length > 0, 6 <= win_f <= length and 1 <= win_ma <= win_f.");
		return err::kInvalidArgument;
	}
	std::fill(Sgn_out, Sgn_out + length, 0.0);
	std::vector<double> incomeBuffer(static_cast<size_t>(win_f), 0.0);
	std::vector<double> outputBuffer(static_cast<size_t>(win_f), 0.0);
	std::vector<double> movingAverageBuffer(static_cast<size_t>(win_ma), 0.0);
	double Sgn_ma_summ = 0;
	double Sgn_out_buff = 0;
	double Out_m = 0.0;
	int w_ma = 0;
	for (int n = 0; n <= length - win_f; n++) {
		for (int i = 0; i < win_f; i++) {
			incomeBuffer[i] = Sgn_in[n + i];
		}

		if (harmonics_clear(incomeBuffer.data(), outputBuffer.data(), &Out_m, win_f) == err::kOk)
				Sgn_out_buff = Out_m;
		else
			Sgn_out_buff = Sgn_in[n + win_f / 2];

		//скользящее среднее
		Sgn_ma_summ -= movingAverageBuffer[w_ma];
		movingAverageBuffer[w_ma] = Sgn_out_buff;
		Sgn_ma_summ += movingAverageBuffer[w_ma];

		w_ma++;
		if (w_ma == win_ma)w_ma = 0;

		const int outputIndex = n + win_f / 2 - win_ma / 2;
		if (outputIndex >= 0 && outputIndex < length)
			Sgn_out[outputIndex] = Sgn_ma_summ / static_cast<double>(win_ma);
	}

	return err::kOk;
}

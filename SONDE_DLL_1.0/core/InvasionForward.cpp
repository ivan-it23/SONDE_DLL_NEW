#include "stdafx.h"

#include <complex>
#include <cmath>

#include "InvasionForward.h"
#include "Constants.h"
#include "SondeState.h"
#include "Logger.h"
#include "SondeIdentity.h"
#include "ErrorState.h"

using namespace std;

// --------------------------------------------------------------------------
// Функции Бесселя мнимого аргумента I0, I1, K0, K1.
// Коэффициенты рядов рассчитываются один раз при первом обращении.
// --------------------------------------------------------------------------

namespace {

// Постоянная Эйлера-Маскерони.
const double eiler = 0.5772156649015328606065120900824024310421;

// Число членов рядов разложения.
const int kInfI = 50;       // ряд для I0, I1
const int kInfKSmall = 50;  // ряд для K0, K1 при |z| < 10
const int kInfKBig = 14;    // асимптотический ряд для K0, K1 при |z| >= 10

struct BesselTables {
	double FxF[101];      // произведение квадратов факториалов
	double FxG[101];      // произведение факториала на гамма-функцию
	double HxFxF[101];    // гармонический ряд / квадрат факториала (для K0)
	double HxH1FxG[101];  // гармонические ряды / (факториал*гамма) (для K1)
	double HankelK0[101]; // символы Ханкеля для K0 при большом аргументе
	double HankelK1[101]; // символы Ханкеля для K1 при большом аргументе
};

BesselTables g_tables = {};
bool g_tables_ready = false;

double factorial(int n) {
	if (n <= 0) return 1.0;
	double result = 1.0;
	for (int i = 1; i <= n; i++) result *= i;
	return result;
}

// Гамма-функция в полуцелых точках: gamma(n + 1/2), с обработкой n < 0.
double gamma05(int n) {
	if (n < 0) return -2.0 * sqrt(PI);
	double g = sqrt(PI);
	for (double i = 0.5; i < n; i++) g *= i;
	return g;
}

void build_bessel_tables() {
	g_tables.FxF[0] = 1.0;
	double fxf = 1.0;
	for (int k = 1; k <= 90; k++) {
		fxf *= static_cast<double>(k) * k;
		g_tables.FxF[k] = fxf;
	}

	g_tables.FxG[0] = 1.0;
	double fxg = 1.0;
	for (int k = 1; k <= 100; k++) {
		fxg *= static_cast<double>(k) * k + k;
		g_tables.FxG[k] = fxg;
	}

	g_tables.HxFxF[0] = 1.0;
	double hxfxf = 1.0;
	double h = 0.0;
	for (int k = 1; k <= 100; k++) {
		hxfxf *= static_cast<double>(k) * k;
		h += 1.0 / k;
		g_tables.HxFxF[k] = h / hxfxf;
	}

	g_tables.HxH1FxG[0] = 1.0;
	double hxfxg = 1.0;
	double h_k = 0.0;
	for (int k = 1; k <= 100; k++) {
		hxfxg *= static_cast<double>(k) * k + k;
		h_k += 1.0 / k;
		double h_k1 = h_k + 1.0 / (k + 1);
		g_tables.HxH1FxG[k] = (h_k + h_k1) / hxfxg;
	}

	for (int k = 0; k < 50; k++) {
		g_tables.HankelK0[k] = pow(PI * factorial(k), -1) * pow(-1.0, k) * gamma05(k) * gamma05(k);
	}
	for (int k = 0; k < 50; k++) {
		g_tables.HankelK1[k] = -pow(PI * factorial(k), -1) * pow(-1.0, k) * gamma05(k + 1) * gamma05(k - 1);
	}

	g_tables_ready = true;
}

void ensure_bessel_tables() {
	if (!g_tables_ready) build_bessel_tables();
}

complex<double> I0(complex<double> z) {
	complex<double> sum = 1.0;
	for (int k = 1; k < kInfI; k++) {
		sum += pow(z / 2.0, 2 * k) / g_tables.FxF[k];
	}
	return sum;
}

complex<double> I1(complex<double> z) {
	complex<double> sum = 0.5 * z;
	for (int k = 1; k < kInfI; k++) {
		sum += pow(0.5 * z, (2 * k) + 1) / g_tables.FxG[k];
	}
	return sum;
}

complex<double> K0_small(complex<double> z) {
	complex<double> sum = 0.0;
	sum -= (log(0.5 * z) + eiler) * I0(z);
	for (int k = 1; k < kInfKSmall; k++) {
		sum += pow(0.5 * z, 2 * k) * g_tables.HxFxF[k];
	}
	return sum;
}

complex<double> K1_small(complex<double> z) {
	complex<double> sum = 0.5 * z;
	for (int k = 1; k < kInfKSmall; k++) {
		sum += pow(0.5 * z, 2 * k + 1) * g_tables.HxH1FxG[k];
	}
	sum *= -0.5;
	sum += 1.0 / z + (log(0.5 * z) + eiler) * I1(z);
	return sum;
}

complex<double> K0_big(complex<double> z) {
	complex<double> sum = 0.0;
	for (int k = 0; k < kInfKBig; k++) {
		sum += g_tables.HankelK0[k] * pow(2.0 * z, -k);
	}
	sum *= sqrt(0.5 * PI / z) * exp(-z);
	return sum;
}

complex<double> K1_big(complex<double> z) {
	complex<double> sum = 0.0;
	for (int k = 0; k < kInfKBig; k++) {
		sum += g_tables.HankelK1[k] * pow(2.0 * z, -k);
	}
	sum *= sqrt(0.5 * PI / z) * exp(-z);
	return sum;
}

complex<double> K0(complex<double> z) {
	return (abs(z) < 10.0) ? K0_small(z) : K0_big(z);
}

complex<double> K1(complex<double> z) {
	return (abs(z) < 10.0) ? K1_small(z) : K1_big(z);
}

} // namespace

// --------------------------------------------------------------------------
// Прямая задача для двухслойной цилиндрической модели.
// --------------------------------------------------------------------------

float Vzz_inf_cyl(SONDE_PARAM param, float Ro_p, float Ro_zp, float rzp) {
	ensure_bessel_tables();

	const double r0 = static_cast<double>(rzp);
	const double W = 2.0 * PI * static_cast<double>(param.f);

	const double dkz1 = 1e-4;
	double dkz2 = 1.0442737824; // 2^1/16
	if (rzp <= 0.1 && param.L1 > 0.8)  dkz2 = 1.002711275;         // 2^1/256
	else if (rzp <= 0.1 && param.L1 <= 0.8) dkz2 = 1.005429901128; // 2^1/128
	else if (rzp > 0.1 && rzp <= 0.2)  dkz2 = 1.010889286;         // 2^1/64
	else if (rzp > 0.2 && rzp <= 0.4)  dkz2 = 1.0218971486;        // 2^1/32

	const double sigma1 = 1.0 / static_cast<double>(Ro_zp);
	const double sigma2 = 1.0 / static_cast<double>(Ro_p);
	const double eps1 = 108.5 * pow(sigma1, 0.35) + 5.0;
	const double eps2 = 108.5 * pow(sigma2, 0.35) + 5.0;

	const complex<double> j(0.0, 1.0);
	const complex<double> k1 = j * W * mu0 * (sigma1 - j * W * eps0 * eps1);
	const complex<double> k2 = j * W * mu0 * (sigma2 - j * W * eps0 * eps2);

	const complex<double> inf_L1 =
		exp(j * sqrt(k1) * static_cast<double>(param.L1)) *
		(1.0 - j * sqrt(k1) * static_cast<double>(param.L1)) / pow(static_cast<double>(param.L1), 3);
	const complex<double> inf_L2 =
		exp(j * sqrt(k1) * static_cast<double>(param.L2)) *
		(1.0 - j * sqrt(k1) * static_cast<double>(param.L2)) / pow(static_cast<double>(param.L2), 3);

	complex<double> integral_L1(0.0, 0.0);
	complex<double> integral_L2(0.0, 0.0);

	for (double kz = dkz1; kz < 0.1; kz += dkz1) {
		const complex<double> x1 = sqrt(pow(kz, 2) - k1);
		const complex<double> x2 = sqrt(pow(kz, 2) - k2);
		const complex<double> K1x2r0 = K1(x2 * r0);
		const complex<double> K0x2r0 = K0(x2 * r0);
		const complex<double> part_b = pow(x1, 2) *
			((x1 * K1x2r0 * K0(x1 * r0) - x2 * K0x2r0 * K1(x1 * r0)) /
			 (x1 * K1x2r0 * I0(x1 * r0) + x2 * K0x2r0 * I1(x1 * r0)));
		integral_L1 += cos(kz * static_cast<double>(param.L1)) * part_b;
		integral_L2 += cos(kz * static_cast<double>(param.L2)) * part_b;
	}
	integral_L1 *= dkz1;
	integral_L2 *= dkz1;

	double dkz = dkz1;
	double kz = 0.1;
	while (kz < 80.0) {
		kz += dkz;
		dkz *= dkz2;
		const complex<double> x1 = sqrt(pow(kz, 2) - k1);
		const complex<double> x2 = sqrt(pow(kz, 2) - k2);
		const complex<double> K1x2r0 = K1(x2 * r0);
		const complex<double> K0x2r0 = K0(x2 * r0);
		const complex<double> part_b = dkz * pow(x1, 2) *
			(x1 * K1x2r0 * K0(x1 * r0) - x2 * K0x2r0 * K1(x1 * r0)) /
			(x1 * K1x2r0 * I0(x1 * r0) + x2 * K0x2r0 * I1(x1 * r0));
		integral_L1 += cos(kz * static_cast<double>(param.L1)) * part_b;
		integral_L2 += cos(kz * static_cast<double>(param.L2)) * part_b;
	}

	integral_L1 /= PI;
	integral_L2 /= PI;
	integral_L1 += inf_L1;
	integral_L2 += inf_L2;

	return static_cast<float>(arg(integral_L2 / integral_L1));
}

int compute_zp_phases(RHO* Ro_src, CAL_SIGNAL* Phase) {
	if (Ro_src == nullptr || Phase == nullptr) {
		SetSondeLastError("ph_smt_zp: не заданы входная структура RHO или выходная CAL_SIGNAL.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("Перед вызовом ph_smt_zp необходимо успешно выполнить sonde_set.");
		return err::kMetrologyNotInitialized;
	}
	if (!IsNeuralLwd4Tx(id)) {
		SetSondeLastError("ph_smt_zp доступна только для приборов LWD и картографа в режиме LWD с четырьмя передатчиками.");
		return err::kUnsupportedType;
	}

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		const float Ro_p = Ro_src->rho_p[freq];
		const float Ro_zp = Ro_src->rho_zp[freq];
		const float R_zp_m = Ro_src->R_zp[freq] / 100.0f; // см -> м
		if (!std::isfinite(Ro_p) || !std::isfinite(Ro_zp) || !std::isfinite(R_zp_m) ||
			Ro_p <= 0.0f || Ro_zp <= 0.0f || R_zp_m <= 0.0f) {
			SetSondeLastError("ph_smt_zp: значения rho_p, rho_zp и R_zp должны быть конечными и положительными на обеих частотах.");
			return err::kInvalidArgument;
		}

		for (int Tx = 0; Tx < config::kMaxTx; Tx++)
			Phase->phase[freq][Tx] = 0.0f;
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++) {
			// зонд считается рабочим только при ненулевой геометрии
			if (param[freq][Tx].L1 > 0.0f && param[freq][Tx].L2 > 0.0f) {
				Phase->phase[freq][Tx] = Vzz_inf_cyl(param[freq][Tx], Ro_p, Ro_zp, R_zp_m);
			}
			else {
				SetSondeLastError("ph_smt_zp: расчёт фазы невозможен, у активного зонда нулевая геометрия L1 или L2.");
				return err::kMetrologyGeometry;
			}
		}
	}

	if (debug == true) {
		Test << "[ZP] ph_smt_zp rho_p=" << Ro_src->rho_p[0]
		     << " rho_zp=" << Ro_src->rho_zp[0]
		     << " R_zp(cm)=" << Ro_src->R_zp[0] << " фазы(мГрад): ";
		for (int freq = 0; freq < config::kFreqCount; freq++)
			for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
				Test << Phase->phase[freq][Tx] * mG << " ";
		Test << endl;
	}

	return err::kOk;
}

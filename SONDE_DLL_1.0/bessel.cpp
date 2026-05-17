#include "stdafx.h"

#include "bessel.h"

#include <complex>

#include <cmath>



int inf_I = 50;

int inf_K_small = 50;

int inf_K_big = 14;

//const double PI = 3.1415927410125732;



double psi(int m) {

	if (m == 0) return 0.0;

	double summ = -eiler;

	for (int i = 1; i < m; i++) {

		summ += 1.0 / i;

	}

	return summ;

}



double H_(int m) {

	if (m == 0) return 0.0;

	double summ = 0;

	for (int i = 1; i <= m; i++) {

		summ += 1.0 / i;

	}

	return summ;

}



double fact(int n) {

	if (n == 0)return 1;

	double summ = 1;

	for (int i = 1; i <= n; i++) {

		summ *= i;

	}

	return summ;

}



double gamma05(int n) {

	if (n < 0) return -2.0*sqrt(PI);

	double g = sqrt(PI);

	for (double i = 0.5f; i < n; i++) {

		g *= i;

	}

	return g;

}



void FxF(int n) {

	pallete.FxF[0] = 1;

	double fxf = 1;

	for (int k = 1; k <= n; k++) {

		fxf *= k * k;

		pallete.FxF[k] = fxf;

	}

}

//произведение факториала на гамма функцию

void FxG(int n) {

	pallete.FxG[0] = 1;

	double fxg = 1;

	for (int k = 1; k <= 100; k++) {

		fxg *= k * k + k;

		pallete.FxG[k] = fxg;

	}

}

//произведение гармонического ряда на квадрат факториала для К0

void HxFxF(int n) {

	pallete.HxFxF[0] = 1;

	double fxf = 1;

	double H = 0;

	for (int k = 1; k <= 100; k++) {

		fxf *= k * k;

		H += 1.0 / k;

		pallete.HxFxF[k] = H / fxf;

	}

}

//произведение гармонических рядов к и к+1 факториала и гамма функции для К1

void HxH1FxG(int n) {

	pallete.HxH1FxG[0] = 1;

	double fxg = 1;

	double H = 0;

	double H1 = 0;

	for (int k = 1; k <= 100; k++) {

		fxg *= k * k + k;

		H += 1.0 / k;

		H1 = H + 1.0 / (k + 1);

		pallete.HxH1FxG[k] = (H + H1) / fxg;

	}

}

// символ Ханкеля для К0 большого

void Hankel_K0(int n) {

	for (int k = 0; k < n; k++) {

		pallete.HankelK0[k] = pow(PI*fact(k), -1)*pow(-1, k)*gamma05(k)*gamma05(k);

	}

}

// символ Ханкеля для К1 большого

void Hankel_K1(int n) {

	for (int k = 0; k < n; k++) {

		pallete.HankelK1[k] = -pow(PI*fact(k), -1)*pow(-1, k)*gamma05(k + 1)*gamma05(k - 1);

	}

}



//функции Бесселя I0 I1 K0 K1 



double J(int a, double z) {

	double summ = 0;

	double dt = PI / 180;

	for (double t = 0; t < PI; t += dt)

		summ += cos(double(a)*t - z * sin(t));

	summ *= (dt / PI);

	return summ;

}



std::complex<double> I0(std::complex<double> z) {



	std::complex<double> summ = 1;

	for (int k = 1; k < inf_I; k++) {

		summ += pow(z / 2.0, 2 * k) / (pallete.FxF[k]);

	}

	return summ;

}



std::complex<double> I1(std::complex<double> z) {

	std::complex<double> summ = 0.5*z;

	for (int k = 1; k < inf_I; k++) {

		summ += pow(0.5*z, (2 * k) + 1) / (pallete.FxG[k]);

	}

	return summ;

}



std::complex<double> K0_small(std::complex<double> z) {

	std::complex<double> summ = 0;

	summ -= (log(0.5*z) + eiler) * I0(z);

	for (int k = 1; k < inf_K_small; k++) {

		summ += pow(0.5*z, 2 * k)*pallete.HxFxF[k];

	}

	return summ;

}



std::complex<double> K1_small(std::complex<double> z) {

	std::complex<double> summ = 0.5*z;

	double FxG = 1;

	for (int k = 1; k < inf_K_small; k++) {

		FxG *= k * k + k;

		summ += pow(0.5*z, 2 * k + 1)*pallete.HxH1FxG[k];

	}

	summ *= -0.5;

	summ += +1.0 / z + (log(0.5*z) + eiler) * I1(z);

	return summ;

}



std::complex<double> K0_big(std::complex<double> z) {

	std::complex<double> summ = 0;

	for (int k = 0; k < inf_K_big; k++) {

		summ += pallete.HankelK0[k] * pow(2.0*z, -k);

	}

	summ *= sqrt(0.5*PI / z)*exp(-z);

	return summ;

}



std::complex<double> K1_big(std::complex<double> z) {

	std::complex<double> summ = 0;

	for (int k = 0; k < inf_K_big; k++) {

		summ += pallete.HankelK1[k] * pow(2.0*z, -k);

	}

	summ *= sqrt(0.5*PI / z)*exp(-z);

	return summ;

}



std::complex<double> K0(std::complex<double> z) {

	if (abs(z) < 10)return K0_small(z);

	if (abs(z) > 10)return K0_big(z);

	return K0_big(z);

}



std::complex<double> K1(std::complex<double> z) {

	if (abs(z) < 10)return K1_small(z);

	if (abs(z) > 10)return K1_big(z);

	return K1_big(z);

}



void create_bessel_pallete() {

	FxF(90);

	FxG(90);

	HxFxF(90);

	HxH1FxG(90);

	Hankel_K0(50);

	Hankel_K1(50);

}



void create_Geo_bessel_pallete(float *L, int N_sonde) {

	for (int sonde = 0; sonde < N_sonde; sonde++) {

		double  dl2 = pow(2, 1.0 / 64.0);//2^1/64

		double dl = 1e-4;

		int n = 0;

		double l = 0.1;

		for (double l = dl; l < 0.1; l += dl) {

			pallete.Vzx_bessel_J0lL[sonde][n] = J(0, l*L[sonde]);

			pallete.Vzx_bessel_J1lL[sonde][n] = J(1, l*L[sonde]);

			pallete.Vzx_bessel_J2lL[sonde][n] = J(2, l*L[sonde]);

			n++;

		}

		while (l < 160) {

			pallete.Vzx_bessel_J0lL[sonde][n] = J(0, l*L[sonde]);

			pallete.Vzx_bessel_J1lL[sonde][n] = J(1, l*L[sonde]);

			pallete.Vzx_bessel_J2lL[sonde][n] = J(2, l*L[sonde]);

			l += dl;

			dl *= dl2;

			n++;

		}

	}



}



void create_Vzz_3coil_bessel_pallete(SONDE_PARAM *param, int N_sonde) {

	for (int sonde = 0; sonde < N_sonde; sonde++) {

		double  dl2 = pow(2, 1.0 / 64.0);//2^1/64

		double dl = 1e-4;

		int n = 0;

		double l = 0.1;

		for (double l = dl; l < 0.1; l += dl) {

			pallete.Vzz_3coil_bessel_J0lL1[sonde][n] = J(0, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J1lL1[sonde][n] = J(1, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J2lL1[sonde][n] = J(2, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J0lL2[sonde][n] = J(0, l*param[sonde].L2);

			pallete.Vzz_3coil_bessel_J1lL2[sonde][n] = J(1, l*param[sonde].L2);

			pallete.Vzz_3coil_bessel_J2lL2[sonde][n] = J(2, l*param[sonde].L2);

			n++;

		}

		while (l < 160) {

			pallete.Vzz_3coil_bessel_J0lL1[sonde][n] = J(0, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J1lL1[sonde][n] = J(1, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J2lL1[sonde][n] = J(2, l*param[sonde].L1);

			pallete.Vzz_3coil_bessel_J0lL2[sonde][n] = J(0, l*param[sonde].L2);

			pallete.Vzz_3coil_bessel_J1lL2[sonde][n] = J(1, l*param[sonde].L2);

			pallete.Vzz_3coil_bessel_J2lL2[sonde][n] = J(2, l*param[sonde].L2);

			l += dl;

			dl *= dl2;

			n++;

		}

	}



}


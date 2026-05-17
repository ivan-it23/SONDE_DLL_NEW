#include "stdafx.h"

#include "bessel.h"

#include "amp_geo.h"

#include <fstream>

#include <iostream>

#include <iomanip>

#include <windows.h>

#include <vector>



#define _SILENCE_AMP_DEPRECATION_WARNINGS

#include <amp.h>

#include <ppl.h>

#include <amp_math.h>

#include <stdio.h>



using namespace concurrency;

using namespace concurrency::fast_math;

//using namespace concurrency::precise_math;



uint32_t ext_size1 = 3000;

///////////////////////////////////////////////////////////////////////

COMPLEX Plus(COMPLEX a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = a.I + b.I;

	result.Q = a.Q + b.Q;

	return result;

}



COMPLEX Plus(COMPLEX a, float b) restrict(amp) {

	COMPLEX result;

	result.I = a.I + b;

	result.Q = a.Q;

	return result;

}



COMPLEX Plus(float a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = a + b.I;

	result.Q = b.Q;

	return result;

}



COMPLEX Minus(COMPLEX a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = a.I - b.I;

	result.Q = a.Q - b.Q;

	return result;

}



COMPLEX Minus(COMPLEX a, float b) restrict(amp) {

	COMPLEX result;

	result.I = a.I - b;

	result.Q = a.Q;

	return result;

}



COMPLEX Minus(float a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = a - b.I;

	result.Q = -b.Q;

	return result;

}



COMPLEX Mult(COMPLEX a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = a.I * b.I - a.Q * b.Q;

	result.Q = a.Q * b.I + a.I * b.Q;

	return result;

}



COMPLEX Mult(COMPLEX a, float b) restrict(amp) {

	COMPLEX result;

	result.I = a.I * b;

	result.Q = a.Q * b;

	return result;

}



COMPLEX Mult(float a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = b.I * a;

	result.Q = b.Q * a;

	return result;

}



COMPLEX Div(COMPLEX a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = (a.I * b.I + a.Q * b.Q) / (b.I*b.I + b.Q*b.Q);

	result.Q = (a.Q * b.I - a.I * b.Q) / (b.I*b.I + b.Q*b.Q);

	return result;

}



COMPLEX Div(COMPLEX a, float b) restrict(amp) {

	COMPLEX result;

	result.I = a.I / b;

	result.Q = a.Q / b;

	return result;

}



COMPLEX Div(float a, COMPLEX b) restrict(amp) {

	COMPLEX result;

	result.I = (a * b.I) / (b.I*b.I + b.Q*b.Q);

	result.Q = (-a * b.Q) / (b.I*b.I + b.Q*b.Q);

	return result;

}



COMPLEX Sqrt(COMPLEX val) restrict(amp) {

	COMPLEX result;

	result.I = concurrency::precise_math::sqrt((val.I + concurrency::precise_math::sqrt(val.I*val.I + val.Q*val.Q)) / 2.0ff);

	if (val.Q >= 0)

		result.Q = concurrency::precise_math::sqrt((-val.I + concurrency::precise_math::sqrt(val.I*val.I + val.Q*val.Q)) / 2.0ff);

	else if (val.Q < 0)

		result.Q = -concurrency::precise_math::sqrt((-val.I + concurrency::precise_math::sqrt(val.I*val.I + val.Q*val.Q)) / 2.0ff);

	return result;

}



COMPLEX Exp(COMPLEX val) restrict(amp) {

	COMPLEX result;

	result.I = concurrency::precise_math::exp(val.I)*concurrency::precise_math::cos(val.Q);

	result.Q = concurrency::precise_math::exp(val.I)*concurrency::precise_math::sin(val.Q);

	return result;

}

////////////////////////////////////////////////////////////////////////////

concurrency::array_view <BESSEL_J_GEO> bessel_J_geo(3000);



concurrency::array_view <BESSEL_J_3COIL> bessel_J_3coil(2000);



void bessel_J_geo_array_init(GEO_SONDE_PARAM *param, std::vector <int> range) {

	int size = 0;

	for (int sonde : range) {

		//float  dl2 = pow(2, 1.0 / 128.0);//2^1/128

		float  dl2 = 1.00543;

		float dl = 1e-4;

		int n = 0;

		float l = 0.1;



		for (float l = dl; l < 0.1; l += dl) {

			bessel_J_geo[n].J0lL[sonde] = J(0, l*param[sonde].L);

			bessel_J_geo[n].J1lL[sonde] = J(1, l*param[sonde].L);

			bessel_J_geo[n].J2lL[sonde] = J(2, l*param[sonde].L);

			bessel_J_geo[n].l = l;

			n++;

		}



		while (l < 160) {

			bessel_J_geo[n].J0lL[sonde] = J(0, l*param[sonde].L);

			bessel_J_geo[n].J1lL[sonde] = J(1, l*param[sonde].L);

			bessel_J_geo[n].J2lL[sonde] = J(2, l*param[sonde].L);

			bessel_J_geo[n].l = l;

			l += dl;

			dl *= dl2;

			n++;

		}

		size = n + 1;

	}

	std::cout << "size " << size << std::endl;

}



void bessel_J_3coil_array_init(SONDE_PARAM *param, std::vector <int> range) {

	int size = 0;

	for (int sonde : range) {

		//float  dl2 = pow(2, 1.0 / 128.0);//2^1/128

		float  dl2 = 1.0108893;

		float dl = 1e-4;

		int n = 0;

		float l = 0.1;



		for (float l = dl; l < 0.1; l += dl) {



			bessel_J_3coil[n].J0lL1[sonde] = J(0, l*param[sonde].L1);

			bessel_J_3coil[n].J0lL2[sonde] = J(0, l*param[sonde].L2);

			bessel_J_3coil[n].J1lL1[sonde] = J(1, l*param[sonde].L1);

			bessel_J_3coil[n].J1lL2[sonde] = J(1, l*param[sonde].L2);

			bessel_J_3coil[n].J2lL1[sonde] = J(2, l*param[sonde].L1);

			bessel_J_3coil[n].J2lL2[sonde] = J(2, l*param[sonde].L2);

			bessel_J_geo[n].l = l;

			n++;

		}



		while (l < 160) {

			bessel_J_3coil[n].J0lL1[sonde] = J(0, l*param[sonde].L1);

			bessel_J_3coil[n].J0lL2[sonde] = J(0, l*param[sonde].L2);

			bessel_J_3coil[n].J1lL1[sonde] = J(1, l*param[sonde].L1);

			bessel_J_3coil[n].J1lL2[sonde] = J(1, l*param[sonde].L2);

			bessel_J_3coil[n].J2lL1[sonde] = J(2, l*param[sonde].L1);

			bessel_J_3coil[n].J2lL2[sonde] = J(2, l*param[sonde].L2);

			bessel_J_geo[n].l = l;

			l += dl;

			dl *= dl2;

			n++;

		}

		size = n + 1;

	}

	std::cout << "size " << size << std::endl;

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

COMPLEX amp_Vzx_3layer(array_view <const BESSEL_J_GEO> bessel_J, GEO_SONDE_PARAM param, uint32_t N_sonde, const ARGUMENTS * arguments) restrict(amp)

{

	const float  PI = 3.1415927410125732f;

	const float mu0 = 4 * PI*1e-7f;

	// для расчета госигнала - 2^1/128

	COMPLEX result;

	int n = 0;

	float D1 = arguments->D_up;

	float D2 = arguments->D_down;

	float H = D1 + D2;

	float W = 2.0f*PI*param.f;

	float dl = 1e-4;

	//float  dl2 = pow(2, 1.0 / 128.0);//2^1/128

	float  dl2 = 1.00543;

	float sigma_up = 1.0f / arguments->Ro_up;

	float sigma_sonde = 1.0f / arguments->Ro_sonde;

	float sigma_down = 1.0f / arguments->Ro_down;



	float k1 = W * mu0*sigma_up;

	float k2 = W * mu0*sigma_sonde;

	float k3 = W * mu0*sigma_down;



	COMPLEX K1; K1.I = -0.0f; K1.Q = -W * mu0 / (4 * PI);



	COMPLEX integral_l; integral_l.I = 0.0f; integral_l.Q = 0.0f;

	for (float l = dl; l < 0.1f; l += dl) {



		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));



		COMPLEX buff = Minus(Mult(K_H_12, Minus(Exp(Mult(p2, -2.0f*D1)), Mult(K_H_32, exp_2p2h))), Mult(K_H_32, Minus(Exp(Mult(p2, -2.0f*D2)), Mult(K_H_12, exp_2p2h))));

		integral_l = Plus(integral_l, Div(Mult(buff, bessel_J[n].J1lL[N_sonde] * l*l), Minus(1.0f, Mult(K_H_12, Mult(K_H_32, exp_2p2h)))));



		n++;

	}

	integral_l = Mult(integral_l, dl);



	float l = 0.1;

	while (l < 160) {

		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));



		l += dl;

		dl *= dl2;



		COMPLEX buff = Minus(Mult(K_H_12, Minus(Exp(Mult(p2, -2.0f*D1)), Mult(K_H_32, exp_2p2h))), Mult(K_H_32, Minus(Exp(Mult(p2, -2.0f*D2)), Mult(K_H_12, exp_2p2h))));

		integral_l = Plus(integral_l, Div(Mult(buff, bessel_J[n].J1lL[N_sonde] * l*l*dl), Minus(1.0f, Mult(K_H_12, Mult(K_H_32, exp_2p2h)))));



		n++;

	}



	result = Mult(integral_l, K1);

	return result;

}



COMPLEX amp_Vzz_3layer(array_view <const BESSEL_J_GEO> bessel_J, GEO_SONDE_PARAM param, uint32_t N_sonde, const ARGUMENTS * arguments) restrict(amp) {

	const float    PI = 3.1415927410125732f;

	const float mu0 = 4 * PI*1e-7f;

	// для расчета госигнала - 2^1/128

	int n = 0;

	float D1 = arguments->D_up;

	float D2 = arguments->D_down;

	float H = D1 + D2;

	float W = 2.0f*PI*param.f;

	float dl = 1e-4f;

	float  dl2 = 1.00543;



	float sigma_up = 1.0f / arguments->Ro_up;

	float sigma_sonde = 1.0f / arguments->Ro_sonde;

	float sigma_down = 1.0f / arguments->Ro_down;



	float k1 = W * mu0*sigma_up;

	float k2 = W * mu0*sigma_sonde;

	float k3 = W * mu0*sigma_down;



	COMPLEX j; j.I = 0.0f; j.Q = 1.0f;

	COMPLEX jk2; jk2.I = 0.0f; jk2.Q = k2;

	COMPLEX j_sqrt_jk2_L = Mult(Mult(j, Sqrt(jk2)), (float)param.L);



	COMPLEX K1; K1.I = 0.0f; K1.Q = W * mu0 / (4 * PI);



	COMPLEX chlen1 = Div(Mult(Exp(j_sqrt_jk2_L), Minus(1.0f, j_sqrt_jk2_L)), 0.5f*param.L*param.L*param.L);





	COMPLEX integral_l; integral_l.I = 0.0f; integral_l.Q = 0.0f;

	for (float l = dl; l < 0.1; l += dl) {



		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));



		COMPLEX mult_sigma_sonde_p1 = Mult(sigma_sonde, p1);

		COMPLEX mult_sigma_up_p2 = Mult(sigma_up, p2);

		COMPLEX mult_sigma_sonde_p3 = Mult(sigma_sonde, p3);

		COMPLEX mult_sigma_down_p2 = Mult(sigma_down, p2);

		COMPLEX K_E_12 = Div(Minus(mult_sigma_sonde_p1, mult_sigma_up_p2), Plus(mult_sigma_sonde_p1, mult_sigma_up_p2));

		COMPLEX K_E_32 = Div(Minus(mult_sigma_sonde_p3, mult_sigma_down_p2), Plus(mult_sigma_sonde_p3, mult_sigma_down_p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));

		COMPLEX exp_2p2D1 = Exp(Mult(p2, -2.0f*D1));

		COMPLEX exp_2p2D2 = Exp(Mult(p2, -2.0f*D2));

		COMPLEX C_H = Mult(Mult(K_H_12, K_H_32), exp_2p2h);

		COMPLEX C_E = Mult(Mult(K_E_12, K_E_32), exp_2p2h);





		integral_l = Plus(integral_l, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL[N_sonde] + bessel_J[n].J2lL[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL[N_sonde] - bessel_J[n].J2lL[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l));





		n++;

	}

	integral_l = Mult(integral_l, dl);



	float l = 0.1f;

	while (l < 160) {



		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));



		COMPLEX mult_sigma_sonde_p1 = Mult(sigma_sonde, p1);

		COMPLEX mult_sigma_up_p2 = Mult(sigma_up, p2);

		COMPLEX mult_sigma_sonde_p3 = Mult(sigma_sonde, p3);

		COMPLEX mult_sigma_down_p2 = Mult(sigma_down, p2);

		COMPLEX K_E_12 = Div(Minus(mult_sigma_sonde_p1, mult_sigma_up_p2), Plus(mult_sigma_sonde_p1, mult_sigma_up_p2));

		COMPLEX K_E_32 = Div(Minus(mult_sigma_sonde_p3, mult_sigma_down_p2), Plus(mult_sigma_sonde_p3, mult_sigma_down_p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));

		COMPLEX exp_2p2D1 = Exp(Mult(p2, -2.0f*D1));

		COMPLEX exp_2p2D2 = Exp(Mult(p2, -2.0f*D2));

		COMPLEX C_H = Mult(Mult(K_H_12, K_H_32), exp_2p2h);

		COMPLEX C_E = Mult(Mult(K_E_12, K_E_32), exp_2p2h);



		l += dl;

		dl *= dl2;



		integral_l = Plus(integral_l, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL[N_sonde] + bessel_J[n].J2lL[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL[N_sonde] - bessel_J[n].J2lL[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l*dl));



		n++;

	}





	return Mult(Minus(chlen1, integral_l), K1);

}



COMPLEX amp_Geo_3layer(COMPLEX Vzz, COMPLEX Vzx) restrict(amp) {

	return Div(Minus(Vzz, Vzx), Plus(Vzz, Vzx));

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// решение прямой задачи для двухслойной  модели с плоской границей для коррекциии УЭС вблизи соседнего пласта 

//для горизонтальной скважины - сигнал трехкатушечного зонда  для известных Ro_sonde, Ro_up, и расстоянии до границы

COMPLEX amp_3coil_3layer(array_view <const BESSEL_J_3COIL> bessel_J, SONDE_PARAM param, uint32_t N_sonde, const ARGUMENTS * arguments) restrict(amp) {

	const float    PI = 3.1415927410125732f;

	const float mu0 = 4 * PI*1e-7f;

	// для расчета госигнала - 2^1/128

	int n = 0;

	float D1 = arguments->D_up;

	float D2 = arguments->D_down;

	float H = D1 + D2;

	float W = 2.0f*PI*param.f;

	float dl = 1e-4f;

	float  dl2 = 1.00543;



	float sigma_up = 1.0f / arguments->Ro_up;

	float sigma_sonde = 1.0f / arguments->Ro_sonde;

	float sigma_down = 1.0f / arguments->Ro_down;



	float k1 = W * mu0*sigma_up;

	float k2 = W * mu0*sigma_sonde;

	float k3 = W * mu0*sigma_down;



	COMPLEX j; j.I = 0.0f; j.Q = 1.0f;

	COMPLEX jk2; jk2.I = 0.0f; jk2.Q = k2;

	COMPLEX K1; K1.I = 0.0f; K1.Q = W * mu0 / (4 * PI);



	COMPLEX j_sqrt_jk2_L1 = Mult(Mult(j, Sqrt(jk2)), (float)param.L1);

	COMPLEX j_sqrt_jk2_L2 = Mult(Mult(j, Sqrt(jk2)), (float)param.L2);



	COMPLEX chlen1_L1 = Div(Mult(Exp(j_sqrt_jk2_L1), Minus(1.0f, j_sqrt_jk2_L1)), 0.5f*param.L1*param.L1*param.L1);

	COMPLEX chlen1_L2 = Div(Mult(Exp(j_sqrt_jk2_L2), Minus(1.0f, j_sqrt_jk2_L2)), 0.5f*param.L2*param.L2*param.L2);





	COMPLEX integral_l1; integral_l1.I = 0.0f; integral_l1.Q = 0.0f;

	COMPLEX integral_l2; integral_l2.I = 0.0f; integral_l2.Q = 0.0f;

	for (float l = dl; l < 0.1; l += dl) {



		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));



		COMPLEX mult_sigma_sonde_p1 = Mult(sigma_sonde, p1);

		COMPLEX mult_sigma_up_p2 = Mult(sigma_up, p2);

		COMPLEX mult_sigma_sonde_p3 = Mult(sigma_sonde, p3);

		COMPLEX mult_sigma_down_p2 = Mult(sigma_down, p2);

		COMPLEX K_E_12 = Div(Minus(mult_sigma_sonde_p1, mult_sigma_up_p2), Plus(mult_sigma_sonde_p1, mult_sigma_up_p2));

		COMPLEX K_E_32 = Div(Minus(mult_sigma_sonde_p3, mult_sigma_down_p2), Plus(mult_sigma_sonde_p3, mult_sigma_down_p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));

		COMPLEX exp_2p2D1 = Exp(Mult(p2, -2.0f*D1));

		COMPLEX exp_2p2D2 = Exp(Mult(p2, -2.0f*D2));

		COMPLEX C_H = Mult(Mult(K_H_12, K_H_32), exp_2p2h);

		COMPLEX C_E = Mult(Mult(K_E_12, K_E_32), exp_2p2h);





		integral_l1 = Plus(integral_l1, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL1[N_sonde] + bessel_J[n].J2lL1[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL1[N_sonde] - bessel_J[n].J2lL1[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l));



		integral_l2 = Plus(integral_l2, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL2[N_sonde] + bessel_J[n].J2lL2[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL2[N_sonde] - bessel_J[n].J2lL2[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l));

		n++;

	}

	integral_l1 = Mult(integral_l1, dl);

	integral_l2 = Mult(integral_l2, dl);





	float l = 0.1f;

	while (l < 160) {



		COMPLEX p1; p1.I = l * l; p1.Q = -k1; p1 = Sqrt(p1);

		COMPLEX p2; p2.I = l * l; p2.Q = -k2; p2 = Sqrt(p2);

		COMPLEX p3; p3.I = l * l; p3.Q = -k3; p3 = Sqrt(p3);



		COMPLEX K_H_12 = Div(Minus(p1, p2), Plus(p1, p2));

		COMPLEX K_H_32 = Div(Minus(p3, p2), Plus(p3, p2));



		COMPLEX mult_sigma_sonde_p1 = Mult(sigma_sonde, p1);

		COMPLEX mult_sigma_up_p2 = Mult(sigma_up, p2);

		COMPLEX mult_sigma_sonde_p3 = Mult(sigma_sonde, p3);

		COMPLEX mult_sigma_down_p2 = Mult(sigma_down, p2);

		COMPLEX K_E_12 = Div(Minus(mult_sigma_sonde_p1, mult_sigma_up_p2), Plus(mult_sigma_sonde_p1, mult_sigma_up_p2));

		COMPLEX K_E_32 = Div(Minus(mult_sigma_sonde_p3, mult_sigma_down_p2), Plus(mult_sigma_sonde_p3, mult_sigma_down_p2));

		COMPLEX exp_2p2h = Exp(Mult(p2, -2.0f*H));

		COMPLEX exp_2p2D1 = Exp(Mult(p2, -2.0f*D1));

		COMPLEX exp_2p2D2 = Exp(Mult(p2, -2.0f*D2));

		COMPLEX C_H = Mult(Mult(K_H_12, K_H_32), exp_2p2h);

		COMPLEX C_E = Mult(Mult(K_E_12, K_E_32), exp_2p2h);



		l += dl;

		dl *= dl2;



		integral_l1 = Plus(integral_l1, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL1[N_sonde] + bessel_J[n].J2lL1[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL1[N_sonde] - bessel_J[n].J2lL1[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l*dl));



		integral_l2 = Plus(integral_l2, Mult((Plus(Div(Mult(Mult(Mult(j, k2), (bessel_J[n].J0lL2[N_sonde] + bessel_J[n].J2lL2[N_sonde])),

			(Minus(Plus(Mult(K_E_12, exp_2p2D1), Mult(K_E_32, exp_2p2D2)), Mult(2.0f, C_E)))), Mult(Mult(2.0f, p2), Minus(1.0f, C_E))),

			Div(Mult(Mult(p2, (bessel_J[n].J0lL2[N_sonde] - bessel_J[n].J2lL2[N_sonde])),

				Plus(Mult(K_H_12, exp_2p2D1), Plus(Mult(K_H_32, exp_2p2D2), Mult(2.0f, C_H)))), Mult(2.0f, Minus(1.0f, C_H))))), l*dl));

		n++;

	}



	return Div(Minus(chlen1_L2, integral_l2), Minus(chlen1_L1, integral_l1));

}



void amp_Geo_intern(array_view <const BESSEL_J_GEO> bessel_J,

	GEO_SONDE_PARAM param[10],

	array_view<const ARGUMENTS, 1> arguments,

	array_view< DT_OUT_DATA, 1> Out_data,

	std::vector <int> range

) {



	const int range_size = range.size();

	int n_range[10];

	for (int i = 0; i < range_size; i++)

		n_range[i] = range[i];

	//массив номеров зондов, участвующих в обработке, для передачи в ГПУ

	concurrency::array<int, 1> N_range(10, n_range);

	//массив параметров зондов  для передачи в ГПУ

	concurrency::array<GEO_SONDE_PARAM, 1> Param(10, param);

	// размер массива аргументов, загружаемый в ГПУ, для параллельных вычислений idx

	concurrency::extent<1> ext(ext_size1);



	// функция параллельных вычислений на ГПУ

	concurrency::parallel_for_each(

		ext, [&, bessel_J, Out_data, arguments, range_size](index<1> idx) restrict(amp) {

		for (int i = 0; i < range_size; i++)

			Out_data[idx].SGN[N_range[i]] = amp_Geo_3layer(amp_Vzz_3layer(bessel_J, Param[N_range[i]], N_range[i], &arguments[idx]), amp_Vzx_3layer(bessel_J, Param[N_range[i]], N_range[i], &arguments[idx]));

	});

	//out_data.synchronize();

}



int amp_Geo(

	GEO_SONDE_PARAM param[10],

	std::vector <ARGUMENTS> arguments,

	std::vector< DT_OUT_DATA> &out_data,

	std::vector <int> range

) {

	array_view<const ARGUMENTS, 1> Arguments(arguments.size(), arguments);

	array_view<DT_OUT_DATA, 1> Out_data(arguments.size(), out_data);

	ext_size1 = arguments.size();

	amp_Geo_intern(bessel_J_geo, param, Arguments, Out_data, range);

	return 1;

}



void amp_3coil_intern(array_view <const BESSEL_J_3COIL> bessel_J,

	SONDE_PARAM param[10],

	array_view<const ARGUMENTS, 1> arguments,

	array_view< DT_OUT_DATA, 1> Out_data,

	std::vector <int> range

) {



	const int range_size = range.size();

	int n_range[10];

	for (int i = 0; i < range_size; i++)

		n_range[i] = range[i];

	//массив номеров зондов, участвующих в обработке, для передачи в ГПУ

	concurrency::array<int, 1> N_range(10, n_range);

	//массив параметров зондов  для передачи в ГПУ

	concurrency::array<SONDE_PARAM, 1> Param(10, param);

	// размер массива аргументов, загружаемый в ГПУ, для параллельных вычислений idx

	concurrency::extent<1> ext(ext_size1);



	// функция параллельных вычислений на ГПУ

	concurrency::parallel_for_each(

		ext, [&, bessel_J, Out_data, arguments, range_size](index<1> idx) restrict(amp) {

		for (int i = 0; i < range_size; i++)

			Out_data[idx].SGN[N_range[i]] = amp_3coil_3layer(bessel_J, Param[N_range[i]], N_range[i], &arguments[idx]);



	});

}



int amp_3coil(

	SONDE_PARAM param[10],

	std::vector <ARGUMENTS> arguments,

	std::vector< DT_OUT_DATA> &out_data,

	std::vector <int> range

) {

	array_view<const ARGUMENTS, 1> Arguments(arguments.size(), arguments);

	array_view<DT_OUT_DATA, 1> Out_data(arguments.size(), out_data);

	ext_size1 = arguments.size();

	amp_3coil_intern(bessel_J_3coil, param, Arguments, Out_data, range);

	return 1;

}
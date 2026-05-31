#include "stdafx.h"

#include <cmath>
#include <complex>
#include <iostream>

#include "Resistivity.h"
#include "Constants.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "MetrologyLoader.h"
#include "Logger.h"

using namespace std;

// --------------------------------------------------------------------------
// Низкоуровневые преобразования фаза/УЭС.
// --------------------------------------------------------------------------

// фаза от УЭС для бесконечной среды
float dFI(SONDE_PARAM param, float Ro) {
	int sign = 1;
	const double omega = 2 * PI * static_cast<double>(param.f);
	double sigma = 1.0 / static_cast<double>(Ro);
	sigma *= sign;
	const std::complex<double> j(0.0, 1.0);
	std::complex<double> ik = j * sqrt(j * omega * mu0 * (sigma - std::complex<double>(0.0, omega * eps0 * 0.0)));
	std::complex<double> ZC1 = exp(ik*(static_cast<double>(param.L1) - static_cast<double>(param.L2))) *  pow((static_cast<double>(param.L2) / static_cast<double>(param.L1)), 3) * ((1.0 - ik * static_cast<double>(param.L1)) / (1.0 - ik * static_cast<double>(param.L2)));
	return static_cast<float>(-arg(ZC1)*sign);
}

// УЭС от фазы по золотому сечению
float RO_dFI(SONDE_PARAM param, double dfi) {
	int sign = 1;
	dfi *= sign;
	float Ro0;
	float epsilon_ARG = config::kGoldenEpsilon; // точность фазы и амплитуды
	float Ro_0 = config::kRoSolverMin, Ro_max = config::kRoSolverMax; // мин. и макс. значение УЭС для расчёта
	float delta;
	float ro_0 = Ro_0;
	float ro_max = Ro_max;
	// разность фаз
	do {
		float X1 = ro_0 + config::kGoldenFactor*(ro_max - ro_0);
		float X2 = ro_max - config::kGoldenFactor*(ro_max - ro_0);
		float A = static_cast<float>(dfi - dFI(param, X1));
		float B = static_cast<float>(dfi - dFI(param, X2));
		if (fabs(A) > fabs(B)) { ro_0 = X1; }
		else { ro_max = X2; }
		if (A == 0 || B == 0) { delta = 0; }
		else { delta = fabs(A - B); }
	} while (delta > epsilon_ARG);

	Ro0 = (ro_0 + ro_max) / 2;
	ro_0 = Ro_0;
	ro_max = Ro_max;
	return  Ro0*sign;
}

float DFI_bhole(SONDE_PARAM param, float D_bh_mm, float ro_bh) {
	float dfi = 0;
	float r_bh = static_cast<float>(D_bh_mm / 2000.0);
	float r_sonde = param.D_sonde_m / 2.0f;
	r_sonde = config::kSondeRadiusM;
	float sigma_bh = 1.0f / ro_bh;
	std::cout << "r_sonde " << r_sonde << " r_bh " << r_bh << " sigma_bh " << sigma_bh << " param.f " << param.f << endl;
	float omega = static_cast<float>(2.0 * PI * param.f);
	float L = (param.L1 + param.L2) / 2; float dL = param.L2 - param.L1;
	float P12 = static_cast<float>((omega*mu0* param.L1* param.L1* param.L1) / 4.0);
	float P22 = static_cast<float>((omega*mu0* param.L2* param.L2* param.L2) / 4.0);

	float sgn_bh = 0.0;
	float  dz = 0.01f, dr = 0.001f;
	for (float r = r_sonde; r <= r_bh; r += dr) {
		float QL1 = 0, QL2 = 0;
		for (float z = -5; z < 5; z += dz) {
			float r00 = static_cast<float>(sqrt(pow((z - L), 2) + pow(r, 2)));
			float r01 = static_cast<float>(sqrt(pow((z - dL / 2), 2) + pow(r, 2)));
			float r11 = static_cast<float>(sqrt(pow((z + dL / 2), 2) + pow(r, 2)));
			QL1 += static_cast<float>((P12*(pow(r, 3) * dr * dz)) / pow(r00*r01, 3));
			QL2 += static_cast<float>((P22*(pow(r, 3) * dr * dz)) / pow(r00*r11, 3));
		}
		const std::complex<float> j(0.0f, 1.0f);
		sgn_bh += std::arg((1.0f + j * QL2 * sigma_bh) / (1.0f + j * QL1 * sigma_bh));
	}
	return(sgn_bh);
}

// --------------------------------------------------------------------------
// Экспортируемые функции коррекции фаз и УЭС.
// --------------------------------------------------------------------------

extern "C" __declspec(dllexport) int borehole_offset(float ro_bh, int D_bhole_mm) {
	if (D_bhole_mm != 0 && ro_bh != 0) {
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				dfi_bh[freq][Tx] = DFI_bhole(param[freq][Tx], static_cast<float>(D_bhole_mm), ro_bh);
			}
		}
	}
	else {
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				dfi_bh[freq][Tx] = 0;
			}
		}
	}
	if (debug == true) {
		Test << "dfi_bh ";
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				Test << dfi_bh[freq][Tx] * mG << " ";
			}
		}
		Test << endl;
	}
	return 0;
}

//ok вводим фазу и требуемое УЭС для данной точки и получаем необходимую фазовую поправку
extern "C" __declspec(dllexport) int ph_shift_smt_ph(PHASE *Phase, Ro *Ro_need, PHASE *Phase_shift) {
	int result = 0;
	float dfi_ro_need[2][5];
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_need->Ro[freq][Tx] > 0.00000001 && Phase->Phase[freq][Tx] != 0.0f) {
				dfi_ro_need[freq][Tx] = dFI(param[freq][Tx], Ro_need->Ro[freq][Tx]);
				Phase_shift->Phase[freq][Tx] = Phase->Phase[freq][Tx] - dfi_ro_need[freq][Tx];
			}
			else Phase_shift->Phase[freq][Tx] = 0;
		}
	}
	return 0;
}

//ok вводим УЭС и требуемое УЭС для данной точки и получаем необходимую фазовую поправку
extern "C" __declspec(dllexport) int ph_shift_smt_ro(Ro *Ro_calk, Ro *Ro_need, PHASE *Phase_shift) {
	int result = 0;
	float dfi_ro_calk[2][5] = { 0, };
	float dfi_ro_need[2][5] = { 0, };
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_need->Ro[freq][Tx] != 0.0f && Ro_calk->Ro[freq][Tx] != 0.0f) {
				dfi_ro_calk[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
				dfi_ro_need[freq][Tx] = dFI(param[freq][Tx], Ro_need->Ro[freq][Tx]);
				Phase_shift->Phase[freq][Tx] = dfi_ro_calk[freq][Tx] - dfi_ro_need[freq][Tx];
			}
			else Phase_shift->Phase[freq][Tx] = 0;
		}
	}
	return 0;
}

//по полученному и желаемому УЭС в опорной точке получаем для полученного в любой другой точке УЭС скорректированное (Ro_required)
//ok Функция работает без sonde_set
extern "C" __declspec(dllexport) int ro_corr_ref_point(void *Metrology, Ro *Ro_calk_ref_point, Ro *Ro_need_ref_point, Ro *Ro_calk, Ro *Ro_required) {
	SONDE_PARAM param[2][5] = { 0, };

	GP_METROLOGY metrology = {};
	uint32_t signature = 0;
	int read_result = read_metrology_file((const char*)Metrology, &metrology, &signature);
	if (read_result != err::kOk)
		return read_result;

	ID id = get_sonde_id(signature);
	if (id.type == LWD_4Tx_NEW || id.type == LWD_4Tx || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
		// param заполняется локально; Air сохраняет историческое поведение записи в глобальное состояние
		fill_sonde_params(metrology, param, Air);
	}

	int result = 0;
	float dfi_ro_calk_ref_point[2][5] = { 0, };
	float dfi_ro_need_ref_point[2][5] = { 0, };
	float dfi_ro_calk[2][5] = { 0, };
	float dfi_ro_required[2][5] = { 0, };
	float Phase_shift[2][5] = { 0, };
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_need_ref_point->Ro[freq][Tx] != 0) {
				dfi_ro_calk_ref_point[freq][Tx] = dFI(param[freq][Tx], Ro_calk_ref_point->Ro[freq][Tx]);
				dfi_ro_need_ref_point[freq][Tx] = dFI(param[freq][Tx], Ro_need_ref_point->Ro[freq][Tx]);
				dfi_ro_calk[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
				Phase_shift[freq][Tx] = dfi_ro_calk_ref_point[freq][Tx] - dfi_ro_need_ref_point[freq][Tx];
				dfi_ro_required[freq][Tx] = dfi_ro_calk[freq][Tx] - Phase_shift[freq][Tx];
				Ro_required->Ro[freq][Tx] = RO_dFI(param[freq][Tx], dfi_ro_required[freq][Tx]);
			}
			else Phase_shift[freq][Tx] = 0;
		}
	}

	return 0;
}

//ok из УЭС получаем симметризованные фазы нужно для операции "КАРАНДАШ"!!!
extern "C" __declspec(dllexport) int ph_smt_ro(Ro *Ro_calk, PHASE *Phase) {
	int result = 0;
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_calk->Ro[freq][Tx] > 0.0f)
				Phase->Phase[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
			else Phase->Phase[freq][Tx] = config::kInvalidPhase;
		}
	}
	return 0;
}

//exp//
//УЭС от фазы по золотому сечению просто и без учета скважины и зоны проникновения
extern "C" __declspec(dllexport) int calculate_Rho_Doll_GR(PHASE *Phase, Ro *Ro_3c) {
	//Golden Ratio
	int result = 0;
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Ro_3c->Ro[freq][Tx] = RO_dFI(param[freq][Tx], Phase->Phase[freq][Tx]);
		}
	}

	if (debug == true) {
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				Test << Ro_3c->Ro[freq][Tx] << " ";
			}
		}
		Test << endl;
	}
	return result;
}

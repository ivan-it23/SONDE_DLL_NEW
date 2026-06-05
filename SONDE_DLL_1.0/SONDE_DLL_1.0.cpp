// SONDE_DLL_1.0.cpp
// Фасад-оркестратор экспортируемого API библиотеки SONDE_DLL.
// Содержит функции инициализации (sonde_set), основного расчёта УЭС с
// нейросетевым предиктором (calculate_Rho_AF) и управления отладкой (debug_mode).
// Остальные экспортируемые функции вынесены в тематические модули:
//   - извлечение/симметризация фаз .................. PhaseProcessor.cpp
//   - коррекция фаз/УЭС ............................. Resistivity.cpp
//   - подавление спиральной помехи ................. AntiSpiral.cpp
// Палеточные методы расчёта вынесены в архив (archive/).

#include "stdafx.h"

#include <iomanip>
#include <vector>

#include "Constants.h"
#include "Types.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "MetrologyLoader.h"
#include "NeuroPredictor.h"
#include "Resistivity.h"
#include "Logger.h"

using namespace std;

// Инициализация: загрузка метрологии для типа прибора и подготовка нейросети.
extern "C" __declspec(dllexport) int sonde_set(void *Metrology, const char *Pallete_dir) {
	int result = err::kOk;

	// чтение файла метрологии
	GP_METROLOGY metrology = {};
	uint32_t signature = 0;
	int read_result = read_metrology_file((const char*)Metrology, &metrology, &signature);
	if (read_result != err::kOk)
		return read_result;

	global_signature = signature;
	id = get_sonde_id(signature);


	if (id.type != LWD_4Tx_NEW && id.type != LWD_4Tx) {
		if (debug == true) Test << "sonde_set unsupported tool type " << id.type << endl;
		return err::kUnsupportedType;
	}

	fill_sonde_params(metrology, param, Air);

	for (int Tx = 0; Tx < 5; Tx++) {
		Test << "sonde_set " << Tx << "  L1 " << metrology.L1[Tx] << " L2 " << metrology.L2[Tx] << endl;
	}
	Test << " f_400 " << metrology.F[_400_kGz] << " f_2000 " << metrology.F[_2000_kGz] << endl;
	Test << "AIR ";
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Test << Air[freq][Tx] * mG << " ";
		}
	}
	Test << endl;

	Test << "param L1 ";
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Test << param[freq][Tx].L1 << " ";
		}
	}
	Test << endl;

	Test << "param L2 ";
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Test << param[freq][Tx].L2 << " ";
		}
	}
	Test << endl;

	Test << "param f ";
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Test << param[freq][Tx].f << " ";
		}
	}
	Test << endl;

	if (debug == true) {
		Test << std::dec << " tool_type " << id.type << " tool_N_Tx " << id.N_Tx << " tool_mod " << id.mod << " tool_number " << id.number << endl;
	}

	// инициализация нейросетевого предиктора (NEURO_TEST.dll)
	int neuro_result = neuro_init();
	if (neuro_result != err::kOk)
		return neuro_result;

	return result;
}

//отлажено без зоны проникновения
extern "C" __declspec(dllexport) int calculate_Rho_AF(PHASE *Phase, Ro *Ro_3c, float ro_bh, int D_bhole_mm, int pz_400, int pz_2000, SERVICE *service) {
	int result = 0;
	vector <int> range[2]; //описание группы зондов, участвующих в расчете ЗП
	ZP Zp[2] = { 0.0f, };  //результат для ЗП
	float phase[2][5];     //фазы
	float dfi_bh[2][5] = { 0.0f, }; //фазы компенсации влияния скважины

	//пронуляем
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			dfi_bh[freq][Tx] = 0.0f;
			Ro_3c->Ro[freq][Tx] = 0.0f;
		}
	}
	Ro_3c->Ro_p[0] = 0.0f; Ro_3c->Ro_zp[0] = 0.0f; Ro_3c->R_zp[0] = 0.0f;
	Ro_3c->Ro_p[1] = 0.0f; Ro_3c->Ro_zp[1] = 0.0f; Ro_3c->R_zp[1] = 0.0f;


	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			phase[freq][Tx] = Phase->Phase[freq][Tx] - dfi_bh[freq][Tx];
		}
	}

	// поддерживаются LWD_4Tx_NEW и LWD_4Tx
	if (id.type != LWD_4Tx_NEW && id.type != LWD_4Tx) {
		return err::kUnsupportedType;
	}

	// УЭС по зондам (4 передатчика) методом золотого сечения
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 4; Tx++) {
			Ro_3c->Ro[freq][Tx] = RO_dFI(param[freq][Tx], Phase->Phase[freq][Tx]);
		}
	}

	// нейросетевой расчёт параметров зоны проникновения (8 фаз -> Ro_p, Ro_zp, R_zp)
	if (neuro_available()) {
		float raw_inputs[config::kNeuroInputCount];
		for (int Tx = 0; Tx < 4; Tx++) {
			raw_inputs[Tx] = phase[0][Tx] * Grad;
			raw_inputs[Tx + 4] = phase[1][Tx] * Grad;
		}
		if (debug == true) {
			Test << "[NEURO] sym_phases_400: ";
			for (int Tx = 0; Tx < 4; Tx++) Test << phase[0][Tx] << " ";
			Test << "sym_phases_2000: ";
			for (int Tx = 0; Tx < 4; Tx++) Test << phase[1][Tx] << " ";
			Test << "raw_inputs: ";
			for (int i = 0; i < config::kNeuroInputCount; i++) Test << raw_inputs[i] << " ";
			Test << endl;
		}
		float out_results[config::kNeuroOutputCount] = { 0.0f };
		int neuro_result = neuro_predict(raw_inputs, out_results);
		if (neuro_result == 0) {
			if (debug == true) {
				Test << "[NEURO] predict OK: Ro_p=" << out_results[0]
				     << " Ro_zp=" << out_results[1]
				     << " R_zp=" << out_results[2] << endl;
			}
			// out[0]->Ro_p, out[1]->Ro_zp, out[2]->R_zp
			for (int freq = 0; freq < 2; freq++) {
				Ro_3c->Ro_p[freq] = out_results[0];
				Ro_3c->Ro_zp[freq] = out_results[1];
				Ro_3c->R_zp[freq] = out_results[2];
			}
			service->delta_percent_min[0] = 0.0f;
			service->delta_percent_min[1] = 0.0f;
			service->delta_percent_start[0] = 0.0f;
			service->delta_percent_start[1] = 0.0f;
		}
		else {
			if (debug == true) {
				Test << "calculate_Rho_AF neuro predict failed, code " << neuro_result << endl;
				if (neuro_last_error())
					Test << "neuro error: " << neuro_last_error() << endl;
			}
			return err::kNeuroPredictFailed;
		}
	}
	else {
		if (debug == true) Test << "calculate_Rho_AF neuro predictor not initialized" << endl;
		return err::kNeuroNotInitialized;
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

//ok
extern "C" __declspec(dllexport) void debug_mode(bool Debug) {
	if (Debug == true) debug = true;
	if (Debug == false) debug = false;
	Test << " debug = " << debug << endl;
}

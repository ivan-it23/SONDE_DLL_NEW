// SONDE_DLL_1.0.cpp
// Фасад-оркестратор экспортируемого API библиотеки SONDE_DLL.
// Содержит функции инициализации (sonde_set), основного расчёта УЭС с
// нейросетевым предиктором (calculate_Rho_AF) и управления отладкой (debug_mode).
// Остальные экспортируемые функции вынесены в тематические модули:
//   - извлечение/симметризация фаз .................. PhaseProcessor.cpp
//   - коррекция фаз/УЭС ............................. Resistivity.cpp
//   - подавление спиральной помехи ................. AntiSpiral.cpp
// Палеточная логика не входит в актуальную DLL; расчёт зоны проникновения выполняет нейросеть.

#include "stdafx.h"

#include <iomanip>
#include <vector>
#include <cmath>

#include "Constants.h"
#include "Types.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "MetrologyLoader.h"
#include "NeuroPredictor.h"
#include "Resistivity.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

// Инициализация: загрузка метрологии для типа прибора и подготовка нейросети.
extern "C" __declspec(dllexport) int sonde_set(void *Metrology, const char *reserved) {
	(void)reserved; // сохранён только для совместимости существующего ABI
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();

	GP_METROLOGY metrology = {};
	uint32_t signature = 0;
	int read_result = read_metrology_file((const char*)Metrology, &metrology, &signature);
	if (read_result != err::kOk)
		return read_result;

	const ToolCapabilities capabilities = GetToolCapabilities(signature);
	SONDE_PARAM candidateParam[2][5] = {};
	float candidateAir[2][5] = {};
	float candidateAirAttDb[2][5] = {};
	fill_sonde_params(metrology, candidateParam, candidateAir, candidateAirAttDb);

	// Нейросеть инициализируется только для актуальной структуры LWD 4Tx
	// (включая картограф в режиме LWD). Остальные типы могут использовать
	// общие функции разбора/симметризации без нейросетевого предиктора.
	if (capabilities.neural) {
		int neuro_result = neuro_init(capabilities.identity.type);
		if (neuro_result != err::kOk)
			return neuro_result;
	}

	CommitSondeState(
		metrology,
		capabilities.identity,
		capabilities.activeTx,
		candidateParam,
		candidateAir,
		candidateAirAttDb);

	if (debug == true) {
		Test << std::dec << "sonde_set signature " << signature
			<< " tool_type " << id.type
			<< " tool_N_Tx " << id.N_Tx
			<< " tool_mod " << id.mod
			<< " tool_number " << id.number
			<< " Rx_Position " << current_metrology.Rx_Position << endl;
		for (uint32_t tx = 0; tx < global_active_tx; ++tx)
			Test << "sonde_set T" << (tx + 1) << " L1 " << metrology.L1[tx]
				<< " L2 " << metrology.L2[tx] << endl;
	}

	return err::kOk;
}

// Расчёт фазовых УЭС по зондам и параметров зоны проникновения нейросетью.
// Коррекция за скважину к входам нейросети не применяется: модель обучена без учёта зоны проникновения.
// Амплитудные УЭС (rho_att) здесь не заполняются — для них служит calculate_rho.
extern "C" __declspec(dllexport) int calculate_Rho_AF(CAL_SIGNAL *cal_signal, RHO *Ro_3c, float ro_bh, int D_bhole_mm, int pz_400, int pz_2000, SERVICE *service) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	(void)ro_bh;
	(void)D_bhole_mm;
	(void)pz_400;
	(void)pz_2000;
	if (!cal_signal || !Ro_3c || !service) {
		SetSondeLastError("calculate_Rho_AF requires non-null CAL_SIGNAL, RHO and SERVICE pointers.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("sonde_set must complete successfully before calculate_Rho_AF.");
		return err::kMetrologyNotInitialized;
	}

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++)
			Ro_3c->rho_ph[freq][Tx] = 0.0f;
		Ro_3c->rho_p[freq] = 0.0f;
		Ro_3c->rho_zp[freq] = 0.0f;
		Ro_3c->R_zp[freq] = 0.0f;
		service->delta_percent_min[freq] = 0.0f;
		service->delta_percent_start[freq] = 0.0f;
	}

	// Current neural model supports LWD tools with 4 transmitters.
	if (!IsNeuralLwd4Tx(id)) {
		SetSondeLastError("Neural calculation is supported only for LWD 4Tx and Cartograph LWD-mode 4Tx tools.");
		return err::kUnsupportedType;
	}

	// Фазовые УЭС по зондам (4 передатчика) методом золотого сечения.
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++) {
			if (!std::isfinite(cal_signal->phase[freq][Tx])) {
				SetSondeLastError("calculate_Rho_AF received a non-finite active phase value.");
				return err::kInvalidArgument;
			}
			Ro_3c->rho_ph[freq][Tx] = RO_ARG(param[freq][Tx], cal_signal->phase[freq][Tx]);
		}
	}

	// нейросетевой расчёт параметров зоны проникновения (8 фаз -> rho_p, rho_zp, R_zp)
	if (neuro_available()) {
		float raw_inputs[config::kNeuroInputCount];
		for (int Tx = 0; Tx < config::kNeuroInputCount / config::kFreqCount; Tx++) {
			raw_inputs[Tx] = cal_signal->phase[0][Tx] * Grad;
			raw_inputs[Tx + 4] = cal_signal->phase[1][Tx] * Grad;
		}
		if (debug == true) {
			Test << "[NEURO] sym_phases_400: ";
			for (int Tx = 0; Tx < 4; Tx++) Test << cal_signal->phase[0][Tx] << " ";
			Test << "sym_phases_2000: ";
			for (int Tx = 0; Tx < 4; Tx++) Test << cal_signal->phase[1][Tx] << " ";
			Test << "raw_inputs: ";
			for (int i = 0; i < config::kNeuroInputCount; i++) Test << raw_inputs[i] << " ";
			Test << endl;
		}
		float out_results[config::kNeuroOutputCount] = { 0.0f };
		int neuro_result = neuro_predict(raw_inputs, out_results);
		if (neuro_result == err::kOk) {
			// NEURO_TEST.dll возвращает: out[0] = r_inv (м), out[1] = rho_inv (Ом·м), out[2] = rho_form (Ом·м).
			// R_zp хранится в структуре RHO в сантиметрах; ph_smt_zp переводит обратно в метры.
			const float r_inv_m = out_results[0];
			const float rho_inv = out_results[1];
			const float rho_form = out_results[2];
			if (!std::isfinite(r_inv_m) || !std::isfinite(rho_inv) || !std::isfinite(rho_form) ||
				r_inv_m <= 0.0f || rho_inv <= 0.0f || rho_form <= 0.0f) {
				SetSondeLastError("Neural predictor returned non-finite or non-positive physical parameters.");
				return err::kNeuroPredictFailed;
			}
			const float r_inv_cm = r_inv_m * 100.0f;
			if (debug == true) {
				Test << "[NEURO] predict raw: r_inv_m=" << r_inv_m
				     << " rho_inv=" << rho_inv
				     << " rho_form=" << rho_form << endl;
				Test << "[NEURO] mapped: rho_p=" << rho_form
				     << " rho_zp=" << rho_inv
				     << " R_zp_cm=" << r_inv_cm << endl;
			}
			for (int freq = 0; freq < config::kFreqCount; freq++) {
				Ro_3c->rho_p[freq] = rho_form;
				Ro_3c->rho_zp[freq] = rho_inv;
				Ro_3c->R_zp[freq] = r_inv_cm;
			}
		}
		else {
			if (debug == true) {
				Test << "calculate_Rho_AF neuro predict failed, code " << neuro_result << endl;
				if (neuro_last_error())
					Test << "neuro error: " << neuro_last_error() << endl;
			}
			if (GetSondeLastError()[0] == '\0') {
				std::string detail = "Neural prediction failed.";
				if (neuro_last_error()) detail += std::string(" Runtime: ") + neuro_last_error();
				SetSondeLastError(detail);
			}
			return err::kNeuroPredictFailed;
		}
	}
	else {
		SetSondeLastError("Neural predictor is not initialized for the current signature.");
		if (debug == true) Test << "calculate_Rho_AF neuro predictor not initialized" << endl;
		return err::kNeuroNotInitialized;
	}

	if (debug == true) {
		for (int freq = 0; freq < config::kFreqCount; freq++) {
			for (uint32_t Tx = 0; Tx < global_active_tx; Tx++) {
				Test << Ro_3c->rho_ph[freq][Tx] << " ";
			}
		}
		Test << endl;
	}

	return err::kOk;
}

extern "C" __declspec(dllexport) void debug_mode(bool Debug) {
	logger::set_enabled(Debug);
}

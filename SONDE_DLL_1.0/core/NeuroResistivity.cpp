#include "stdafx.h"

#include <cmath>
#include <string>

#include "NeuroResistivity.h"
#include "Constants.h"
#include "SondeState.h"
#include "SondeIdentity.h"
#include "Resistivity.h"
#include "NeuroPredictor.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

int compute_true_rho_neuro(CAL_SIGNAL* cal_signal, RHO* Ro_3c, SERVICE* service) {
	if (!cal_signal || !Ro_3c || !service) {
		SetSondeLastError("calculate_true_rho_neuro: не заданы указатели на CAL_SIGNAL, RHO или SERVICE.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("Перед вызовом calculate_true_rho_neuro необходимо успешно выполнить sonde_set.");
		return err::kMetrologyNotInitialized;
	}

	// пронуляем
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++)
			Ro_3c->rho_ph[freq][Tx] = 0.0f;
		Ro_3c->rho_p[freq] = 0.0f;
		Ro_3c->rho_zp[freq] = 0.0f;
		Ro_3c->R_zp[freq] = 0.0f;
		service->delta_percent_min[freq] = 0.0f;
		service->delta_percent_start[freq] = 0.0f;
	}

	if (!IsNeuralLwd4Tx(id)) {
		SetSondeLastError("Нейросетевой расчёт поддерживается только для приборов LWD и картографа в режиме LWD с четырьмя передатчиками.");
		return err::kUnsupportedType;
	}

	// УЭС для однородной среды по золотому сечению для всех активных зондов
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++) {
			if (!std::isfinite(cal_signal->phase[freq][Tx])) {
				SetSondeLastError("calculate_true_rho_neuro: получено нечисловое значение фазы активного зонда.");
				return err::kInvalidArgument;
			}
			Ro_3c->rho_ph[freq][Tx] = RO_ARG(param[freq][Tx], cal_signal->phase[freq][Tx]);
		}
	}

	// разброс от среднего в % по УЭС для однородной среды по группе активных зондов
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		float ro_sum = 0.0f;
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
			ro_sum += Ro_3c->rho_ph[freq][Tx];
		if (global_active_tx > 0 && ro_sum > 0.0f) {
			const float ro_sr = ro_sum / static_cast<float>(global_active_tx);
			float delta_sr = 0.0f;
			for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
				delta_sr += std::fabs(Ro_3c->rho_ph[freq][Tx] - ro_sr);
			service->delta_percent_start[freq] =
				100.0f * (delta_sr / static_cast<float>(global_active_tx)) / ro_sr;
		}
	}

	if (!neuro_available()) {
		SetSondeLastError("Нейросетевой предиктор не инициализирован для текущей сигнатуры прибора.");
		if (debug == true) Test << "calculate_true_rho_neuro: нейросетевой предиктор не инициализирован" << endl;
		return err::kNeuroNotInitialized;
	}

	// на вход сети подаются симметризованные фазы 4 зондов на двух частотах, в градусах
	float raw_inputs[config::kNeuroInputCount];
	for (int Tx = 0; Tx < config::kNeuroInputCount / config::kFreqCount; Tx++) {
		raw_inputs[Tx] = cal_signal->phase[0][Tx] * Grad;
		raw_inputs[Tx + 4] = cal_signal->phase[1][Tx] * Grad;
	}
	if (debug == true) {
		Test << "[NEURO] симметризованные фазы 400: ";
		for (int Tx = 0; Tx < 4; Tx++) Test << cal_signal->phase[0][Tx] << " ";
		Test << "симметризованные фазы 2000: ";
		for (int Tx = 0; Tx < 4; Tx++) Test << cal_signal->phase[1][Tx] << " ";
		Test << "входы сети: ";
		for (int i = 0; i < config::kNeuroInputCount; i++) Test << raw_inputs[i] << " ";
		Test << endl;
	}

	float out_results[config::kNeuroOutputCount] = { 0.0f };
	int neuro_result = neuro_predict(raw_inputs, out_results);
	if (neuro_result != err::kOk) {
		if (debug == true) {
			Test << "calculate_true_rho_neuro: сбой предсказания, код " << neuro_result << endl;
			if (neuro_last_error())
				Test << "ошибка нейросети: " << neuro_last_error() << endl;
		}
		if (GetSondeLastError()[0] == '\0') {
			std::string detail = "Сбой нейросетевого предсказания.";
			if (neuro_last_error()) detail += std::string(" Сообщение нейросетевой библиотеки: ") + neuro_last_error();
			SetSondeLastError(detail);
		}
		return err::kNeuroPredictFailed;
	}

	// сеть возвращает out[0] = r_inv (м), out[1] = rho_inv (Ом*м), out[2] = rho_form (Ом*м)
	const float r_inv_m = out_results[0];
	const float rho_inv = out_results[1];
	const float rho_form = out_results[2];
	if (!std::isfinite(r_inv_m) || !std::isfinite(rho_inv) || !std::isfinite(rho_form) ||
		r_inv_m <= 0.0f || rho_inv <= 0.0f || rho_form <= 0.0f) {
		SetSondeLastError("Нейросеть вернула нечисловые или неположительные значения физических параметров.");
		return err::kNeuroPredictFailed;
	}

	// R_zp хранится в структуре RHO в сантиметрах
	const float r_inv_cm = r_inv_m * 100.0f;
	if (debug == true) {
		Test << "[NEURO] выход сети: r_inv_m=" << r_inv_m
		     << " rho_inv=" << rho_inv
		     << " rho_form=" << rho_form << endl;
		Test << "[NEURO] запись в RHO: rho_p=" << rho_form
		     << " rho_zp=" << rho_inv
		     << " R_zp_cm=" << r_inv_cm << endl;
	}
	// параметры зоны проникновения — свойства среды и одинаковы для обеих частот
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		Ro_3c->rho_p[freq] = rho_form;
		Ro_3c->rho_zp[freq] = rho_inv;
		Ro_3c->R_zp[freq] = r_inv_cm;
	}

	if (debug == true) {
		Test << "[RO_ARG] depth=" << cal_signal->Depth << " | УЭС 400 T1-T4: ";
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
			Test << Ro_3c->rho_ph[0][Tx] << " ";
		Test << "| УЭС 2000 T1-T4: ";
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
			Test << Ro_3c->rho_ph[1][Tx] << " ";
		Test << "| Ro_p=" << Ro_3c->rho_p[0] << endl;
	}

	return err::kOk;
}

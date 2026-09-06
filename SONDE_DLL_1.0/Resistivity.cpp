#include "stdafx.h"

#include <cmath>
#include <complex>

#include "Resistivity.h"
#include "Constants.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "MetrologyLoader.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

// --------------------------------------------------------------------------
// Низкоуровневые преобразования сигнал/УЭС.
// --------------------------------------------------------------------------

// Комплексный сигнал зонда (отношение сигналов на приёмниках) для бесконечной
// однородной среды. Коэффициент 0.0000078957 = 2*PI*mu0*1e6/2 в единицах модели.
complex<float> SIGNAL(SONDE_PARAM param, float ro) {
	const float L1 = param.L1;
	const float L2 = param.L2;
	const float f = param.f;
	const float omegamu0sigma = (0.0000078957f * f) / ro;
	const complex<float> j(0.0f, 1.0f);
	const complex<float> ik = j * sqrt(j * omegamu0sigma);
	return exp(ik * (L2 - L1)) * ((1.0f - ik * L2) / (1.0f - ik * L1));
}

// УЭС от фазового сдвига по золотому сечению.
float RO_ARG(SONDE_PARAM param, double dfi) {
	float ro_0 = config::kRoSolverMin;
	float ro_max = config::kRoSolverMax;
	float delta = 0.0f;
	// Верхний предел итераций — защита от зависания при вырожденной геометрии;
	// на реальных данных сходимость наступает за десятки шагов.
	for (int iteration = 0; iteration < 100000; ++iteration) {
		float X1 = ro_0 + config::kGoldenFactor * (ro_max - ro_0);
		float X2 = ro_max - config::kGoldenFactor * (ro_max - ro_0);
		float A = static_cast<float>(dfi - arg(SIGNAL(param, X1)));
		float B = static_cast<float>(dfi - arg(SIGNAL(param, X2)));
		if (fabs(A) > fabs(B)) { ro_0 = X1; }
		else { ro_max = X2; }
		if (A == 0.0f || B == 0.0f) { delta = 0.0f; }
		else { delta = fabs(A - B); }
		if (delta <= config::kGoldenEpsilon)
			break;
	}
	return (ro_0 + ro_max) / 2.0f;
}

// УЭС от затухания в дБ по золотому сечению.
float RO_ATT(SONDE_PARAM param, double att_dB) {
	float ro_0 = config::kRoSolverMin;
	float ro_max = config::kRoAttSolverMax;
	float delta = 0.0f;
	// Целевое затухание переводится из децибел обратно в разы.
	const float target = powf(10.0f, static_cast<float>(att_dB) / 20.0f);
	for (int iteration = 0; iteration < 100000; ++iteration) {
		float X1 = ro_0 + config::kGoldenFactor * (ro_max - ro_0);
		float X2 = ro_max - config::kGoldenFactor * (ro_max - ro_0);
		float A = target - abs(SIGNAL(param, X1));
		float B = target - abs(SIGNAL(param, X2));
		if (fabs(A) > fabs(B)) { ro_0 = X1; }
		else { ro_max = X2; }
		if (A == 0.0f || B == 0.0f) { delta = 0.0f; }
		else { delta = fabs(A - B); }
		if (delta <= config::kGoldenEpsilon)
			break;
	}
	return (ro_0 + ro_max) / 2.0f;
}

// --------------------------------------------------------------------------
// Экспортируемые функции расчёта и коррекции УЭС.
// --------------------------------------------------------------------------

// УЭС по фазе и по затуханию методом золотого сечения, без учёта скважины и
// зоны проникновения.
extern "C" __declspec(dllexport) int calculate_rho(CAL_SIGNAL *cal_signal, RHO *rho) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!cal_signal || !rho) {
		SetSondeLastError("calculate_rho requires non-null CAL_SIGNAL input and RHO output.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("sonde_set must complete successfully before calculate_rho.");
		return err::kMetrologyNotInitialized;
	}
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			rho->rho_ph[freq][Tx] = RO_ARG(param[freq][Tx], cal_signal->phase[freq][Tx]);
			rho->rho_att[freq][Tx] = RO_ATT(param[freq][Tx], cal_signal->att_dB[freq][Tx]);
		}
	}
	if (debug == true) {
		for (int freq = 0; freq < config::kFreqCount; freq++)
			for (uint32_t Tx = 0; Tx < global_active_tx; Tx++)
				Test << rho->rho_ph[freq][Tx] << " ";
		Test << endl;
	}
	return err::kOk;
}

// По вычисленному и желаемому УЭС в опорной точке корректирует УЭС в искомой
// точке (посадка на опорную точку) по обоим каналам. Работает без sonde_set —
// читает собственный файл метрологии.
extern "C" __declspec(dllexport) int rho_corr_ref_point(void *Metrology, RHO *rho_calk_ref_point, RHO *rho_need_ref_point, RHO *rho_calk, RHO *rho_required) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!Metrology || !rho_calk_ref_point || !rho_need_ref_point || !rho_calk || !rho_required) {
		SetSondeLastError("rho_corr_ref_point requires a metrology path and four non-null RHO pointers.");
		return err::kInvalidArgument;
	}
	SONDE_PARAM localParam[2][5] = { 0, };
	float localAir[2][5] = { 0, };
	float localAirAttDb[2][5] = { 0, };

	GP_METROLOGY metrology = {};
	uint32_t signature = 0;
	int read_result = read_metrology_file((const char*)Metrology, &metrology, &signature);
	if (read_result != err::kOk)
		return read_result;

	fill_sonde_params(metrology, localParam, localAir, localAirAttDb);

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			// Фазовый и амплитудный сдвиги в опорной точке.
			float dfi_calk_ref = arg(SIGNAL(localParam[freq][Tx], rho_calk_ref_point->rho_ph[freq][Tx]));
			float att_calk_ref = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_calk_ref_point->rho_att[freq][Tx])));
			float dfi_need_ref = arg(SIGNAL(localParam[freq][Tx], rho_need_ref_point->rho_ph[freq][Tx]));
			float att_need_ref = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_need_ref_point->rho_att[freq][Tx])));
			// Сдвиги в искомой точке.
			float dfi_calk = arg(SIGNAL(localParam[freq][Tx], rho_calk->rho_ph[freq][Tx]));
			float att_calk = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_calk->rho_att[freq][Tx])));
			// Смещение опорной точки и скорректированные сигналы искомой точки.
			float phase_shift = dfi_calk_ref - dfi_need_ref;
			float att_shift = att_calk_ref - att_need_ref;
			float dfi_required = dfi_calk - phase_shift;
			float att_required = att_calk - att_shift;
			rho_required->rho_ph[freq][Tx] = RO_ARG(localParam[freq][Tx], dfi_required);
			rho_required->rho_att[freq][Tx] = RO_ATT(localParam[freq][Tx], att_required);
		}
	}
	return err::kOk;
}

// Из УЭС получаем симметризованные сигналы (фаза + затухание в дБ).
// Нужно для операции "КАРАНДАШ". Вне допустимого диапазона УЭС ставится
// маркер недопустимого значения.
extern "C" __declspec(dllexport) int signal_smt_from_ro(RHO *rho_calk, CAL_SIGNAL *cal_signal) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!rho_calk || !cal_signal) {
		SetSondeLastError("signal_smt_from_ro requires non-null RHO input and CAL_SIGNAL output.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("sonde_set must complete successfully before signal_smt_from_ro.");
		return err::kMetrologyNotInitialized;
	}
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			if (rho_calk->rho_ph[freq][Tx] > 0.0f && rho_calk->rho_ph[freq][Tx] < 7200.0f)
				cal_signal->phase[freq][Tx] = arg(SIGNAL(param[freq][Tx], rho_calk->rho_ph[freq][Tx]));
			else
				cal_signal->phase[freq][Tx] = config::kInvalidPhase;

			if (rho_calk->rho_att[freq][Tx] > 0.0f && rho_calk->rho_att[freq][Tx] < 1200.0f)
				cal_signal->att_dB[freq][Tx] = 20.0f * log10(abs(SIGNAL(param[freq][Tx], rho_calk->rho_att[freq][Tx])));
			else
				cal_signal->att_dB[freq][Tx] = config::kInvalidPhase;
		}
	}
	return err::kOk;
}

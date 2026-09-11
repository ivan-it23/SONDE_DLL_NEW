#include "stdafx.h"

#include <cmath>
#include <complex>

#include "Resistivity.h"
#include "Constants.h"
#include "SondeState.h"
#include "SondeIdentity.h"
#include "Metrology.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

// --------------------------------------------------------------------------
// Низкоуровневые преобразования сигнал/УЭС.
// --------------------------------------------------------------------------

complex<float> SIGNAL(SONDE_PARAM param, float ro) {
	const float L1 = param.L1;
	const float L2 = param.L2;
	const float f = param.f;
	const float omegamu0sigma = (0.0000078957f * f) / ro;
	const complex<float> j(0.0f, 1.0f);
	const complex<float> ik = j * sqrt(j * omegamu0sigma);
	return exp(ik * (L2 - L1)) * ((1.0f - ik * L2) / (1.0f - ik * L1));
}

// УЭС от фазы по золотому сечению.
float RO_ARG(SONDE_PARAM param, double dfi) {
	float ro_0 = config::kRoSolverMin;
	float ro_max = config::kRoSolverMax;
	float delta = 0.0f;
	// Число итераций ограничено сверху: при вырожденной геометрии зонда критерий
	// совпадения фазы недостижим.
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
	// симметризованное затухание переводится обратно из децибелл в разы
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
// Расчёт и коррекция УЭС.
// --------------------------------------------------------------------------

int compute_rho(CAL_SIGNAL* cal_signal, RHO* rho) {
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

int correct_rho_to_ref_point(
	const char* metrologyPath,
	RHO* rho_calk_ref_point,
	RHO* rho_need_ref_point,
	RHO* rho_calk_desired_point,
	RHO* rho_required_desired_point) {
	if (!metrologyPath || !rho_calk_ref_point || !rho_need_ref_point ||
		!rho_calk_desired_point || !rho_required_desired_point) {
		SetSondeLastError("rho_corr_ref_point requires a metrology path and four non-null RHO pointers.");
		return err::kInvalidArgument;
	}
	SONDE_PARAM localParam[2][5] = { 0, };
	float localAir[2][5] = { 0, };
	float localAirAttDb[2][5] = { 0, };

	GP_METROLOGY metrology = {};
	uint32_t signature = 0;
	int read_result = read_metrology_file(metrologyPath, &metrology, &signature);
	if (read_result != err::kOk)
		return read_result;

	fill_sonde_params(metrology, localParam, localAir, localAirAttDb);

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			// фазовый сдвиг и затухание в дБ для вычисленного в опорной точке УЭС
			float dfi_calk_ref = arg(SIGNAL(localParam[freq][Tx], rho_calk_ref_point->rho_ph[freq][Tx]));
			float att_calk_ref = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_calk_ref_point->rho_att[freq][Tx])));
			// фазовый сдвиг и затухание в дБ для требуемого в опорной точке УЭС
			float dfi_need_ref = arg(SIGNAL(localParam[freq][Tx], rho_need_ref_point->rho_ph[freq][Tx]));
			float att_need_ref = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_need_ref_point->rho_att[freq][Tx])));
			// фазовый сдвиг и затухание в дБ для вычисленного в искомой точке УЭС
			float dfi_calk = arg(SIGNAL(localParam[freq][Tx], rho_calk_desired_point->rho_ph[freq][Tx]));
			float att_calk = 20.0f * log10(abs(SIGNAL(localParam[freq][Tx], rho_calk_desired_point->rho_att[freq][Tx])));
			// смещение между вычисленным и требуемым сигналами в опорной точке
			float phase_shift = dfi_calk_ref - dfi_need_ref;
			float att_shift = att_calk_ref - att_need_ref;
			// скорректированный сигнал искомой точки = первоначальный - смещение в опорной точке
			float dfi_required = dfi_calk - phase_shift;
			float att_required = att_calk - att_shift;
			// скорректированное УЭС искомой точки от скорректированного сигнала
			rho_required_desired_point->rho_ph[freq][Tx] = RO_ARG(localParam[freq][Tx], dfi_required);
			rho_required_desired_point->rho_att[freq][Tx] = RO_ATT(localParam[freq][Tx], att_required);
		}
	}
	return err::kOk;
}

int compute_signal_from_rho(RHO* rho_calk, CAL_SIGNAL* cal_signal) {
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
				// затухание переводится в децибеллы
				cal_signal->att_dB[freq][Tx] = 20.0f * log10(abs(SIGNAL(param[freq][Tx], rho_calk->rho_att[freq][Tx])));
			else
				cal_signal->att_dB[freq][Tx] = config::kInvalidPhase;
		}
	}
	return err::kOk;
}

// SondeApi.cpp
// Единственная точка экспорта библиотеки SONDE_DLL. Функции сериализуют доступ
// к состоянию прибора, сбрасывают текст последней ошибки и делегируют расчёт
// модулям каталога core.

#include "stdafx.h"

#include "Constants.h"
#include "Types.h"
#include "SondeState.h"
#include "SondeIdentity.h"
#include "Metrology.h"
#include "SignalCalibration.h"
#include "Resistivity.h"
#include "NeuroResistivity.h"
#include "InvasionForward.h"
#include "DataFile.h"
#include "AntiSpiral.h"
#include "NeuroPredictor.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

// Загружает файл метрологии для типа прибора и, если тип нейросетевой,
// инициализирует предиктор. Состояние прибора заменяется только при успехе.
extern "C" __declspec(dllexport) int sonde_set(void *Metrology) {
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

extern "C" __declspec(dllexport) int get_data_file_info(
	const char *dataPath,
	uint32_t *frameCount,
	int *frameHeaderSize,
	uint32_t *dataSignature) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return scan_data_file(dataPath, frameCount, frameHeaderSize, dataSignature);
}

extern "C" __declspec(dllexport) int get_express_data(void *Data, CAL_SIGNAL *cal_signal, RHO *rho, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return extract_express_data(Data, shift, cal_signal, rho);
}

extern "C" __declspec(dllexport) int get_cal_signal(void *Data, CAL_SIGNAL *cal_signal, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return calibrate_signal(Data, shift, cal_signal);
}

extern "C" __declspec(dllexport) int get_condition(void *Data, uint32_t *condition, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return extract_condition(Data, shift, condition);
}

extern "C" __declspec(dllexport) int simmetry(CAL_SIGNAL *cal_signal_in, CAL_SIGNAL *cal_signal_smt, uint32_t condition) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return symmetrize_signal(cal_signal_in, cal_signal_smt, condition);
}

extern "C" __declspec(dllexport) int calculate_rho(CAL_SIGNAL *cal_signal, RHO *rho) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return compute_rho(cal_signal, rho);
}

extern "C" __declspec(dllexport) int calculate_true_rho_neuro(CAL_SIGNAL *cal_signal, RHO *Ro_3c, SERVICE *service) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return compute_true_rho_neuro(cal_signal, Ro_3c, service);
}

extern "C" __declspec(dllexport) int rho_corr_ref_point(void *Metrology, RHO *rho_calk_ref_point, RHO *rho_need_ref_point, RHO *rho_calk, RHO *rho_required) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return correct_rho_to_ref_point((const char*)Metrology, rho_calk_ref_point, rho_need_ref_point, rho_calk, rho_required);
}

extern "C" __declspec(dllexport) int signal_smt_from_ro(RHO *rho_calk, CAL_SIGNAL *cal_signal) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return compute_signal_from_rho(rho_calk, cal_signal);
}

extern "C" __declspec(dllexport) int ph_smt_zp(RHO *Ro_src, CAL_SIGNAL *Phase) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	return compute_zp_phases(Ro_src, Phase);
}

extern "C" __declspec(dllexport) int anti_spiral(double *Sgn_in, double *Sgn_out, int length, int win_f, int win_ma) {
	ClearSondeLastError();
	return suppress_spiral(Sgn_in, Sgn_out, length, win_f, win_ma);
}

extern "C" __declspec(dllexport) void debug_mode(bool Debug) {
	logger::set_enabled(Debug);
}

// Возвращает текст последней ошибки текущего потока.
extern "C" __declspec(dllexport) const char* sonde_get_last_error() {
	return GetSondeLastError();
}

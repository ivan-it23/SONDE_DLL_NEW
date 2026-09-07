#include "stdafx.h"
#include "SondeState.h"
#include <cstring>

SONDE_PARAM param[2][5] = { 0.0f, };
float Air[2][5] = { 0.0f, };
float Air_att_dB[2][5] = { 0.0f, };
float dfi_bh[2][5] = { 0.0f, };
uint32_t global_signature = 0;
ID id = {};
GP_METROLOGY current_metrology = {};
uint32_t global_active_tx = 0;
uint32_t global_rx_position = 0;
bool sonde_initialized = false;

namespace {
std::recursive_mutex g_sondeStateMutex;
}

// Рекурсивный мьютекс для сериализации доступа к глобальному состоянию прибора.
std::recursive_mutex& SondeStateMutex() {
	return g_sondeStateMutex;
}

// Фиксация состояния прибора после успешной загрузки метрологии: геометрия зондов,
// нули воздуха, идентификатор, число активных передатчиков и Rx_Position.
void CommitSondeState(
	const GP_METROLOGY& metrology,
	const ID& tool,
	uint32_t activeTx,
	const SONDE_PARAM newParam[2][5],
	const float newAir[2][5],
	const float newAirAttDb[2][5]) {
	std::memcpy(param, newParam, sizeof(param));
	std::memcpy(Air, newAir, sizeof(Air));
	std::memcpy(Air_att_dB, newAirAttDb, sizeof(Air_att_dB));
	std::memset(dfi_bh, 0, sizeof(dfi_bh));
	current_metrology = metrology;
	global_signature = metrology.signature;
	id = tool;
	global_active_tx = activeTx;
	global_rx_position = metrology.Rx_Position;
	sonde_initialized = true;
}

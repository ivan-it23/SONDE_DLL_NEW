#include "stdafx.h"
#include "SondeState.h"
#include <cstring>

SONDE_PARAM param[2][5] = { 0.0f, };
float Air[2][5] = { 0.0f, };
float Air_att_dB[2][5] = { 0.0f, };
uint32_t global_signature = 0;
ID id = {};
GP_METROLOGY current_metrology = {};
uint32_t global_active_tx = 0;
bool sonde_initialized = false;

namespace {
std::recursive_mutex g_sondeStateMutex;
}

std::recursive_mutex& SondeStateMutex() {
	return g_sondeStateMutex;
}

// Переносит в глобальное состояние геометрию зондов, нули воздуха, идентификатор
// прибора и число активных передатчиков.
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
	current_metrology = metrology;
	global_signature = metrology.signature;
	id = tool;
	global_active_tx = activeTx;
	sonde_initialized = true;
}

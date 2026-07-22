#pragma once
// SondeState.h
// Разделяемое состояние времени выполнения, формируемое функцией sonde_set и
// используемое расчётными модулями. Имена глобальных объектов сохранены
// идентичными исходным, чтобы минимизировать изменения в логике.

#include "Types.h"
#include <mutex>

// Геометрические и частотные параметры зондов [частота][Tx], заполняются из
// файла метрологии.
extern SONDE_PARAM param[2][5];

// Поправки "нули воздуха" [частота][Tx].
extern float Air[2][5];

// Фазовые поправки за влияние скважины [частота][Tx] (borehole_offset).
extern float dfi_bh[2][5];

// Сигнатура из файла метрологии, полученная в sonde_set.
extern uint32_t global_signature;

// Идентификатор текущего типа прибора.
extern ID id;

extern GP_METROLOGY current_metrology;
extern uint32_t global_active_tx;
extern uint32_t global_rx_position;
extern bool sonde_initialized;

// Сериализует смену метрологии и вычисления над общим ABI-контекстом DLL.
std::recursive_mutex& SondeStateMutex();

void CommitSondeState(
	const GP_METROLOGY& metrology,
	const ID& tool,
	uint32_t activeTx,
	const SONDE_PARAM newParam[2][5],
	const float newAir[2][5]);

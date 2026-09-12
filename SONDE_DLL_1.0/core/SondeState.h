#pragma once
// SondeState.h
// Разделяемое состояние времени выполнения, формируемое функцией sonde_set и
// используемое расчётными модулями.

#include "Types.h"
#include <mutex>

// Геометрические и частотные параметры зондов [частота][Tx] из файла метрологии.
extern SONDE_PARAM param[2][5];

// Фазовые нули воздуха [частота][Tx], рад.
extern float Air[2][5];

// Амплитудные нули воздуха [частота][Tx], дБ.
extern float Air_att_dB[2][5];

// Сигнатура из файла метрологии, полученная в sonde_set.
extern uint32_t global_signature;

// Идентификатор текущего типа прибора.
extern ID id;

extern GP_METROLOGY current_metrology;
extern uint32_t global_active_tx;
extern bool sonde_initialized;

// Сериализует смену метрологии и вычисления над общим ABI-контекстом DLL.
std::recursive_mutex& SondeStateMutex();

// Фиксирует состояние прибора после успешной загрузки метрологии.
void CommitSondeState(
	const GP_METROLOGY& metrology,
	const ID& tool,
	uint32_t activeTx,
	const SONDE_PARAM newParam[2][5],
	const float newAir[2][5],
	const float newAirAttDb[2][5]);

#pragma once
// SondeIdentity.h
// Классификация прибора по сигнатуре: разбор сигнатуры на поля ID и определение
// возможностей прибора (поддержка расчёта, допуск к нейросети, число зондов).

#include "Types.h"

// Декодирует 32-битную сигнатуру кадра/метрологии в идентификатор прибора.
ID get_sonde_id(uint32_t signature);

struct ToolCapabilities {
	ID identity;
	uint8_t activeTx;
	bool supported;
	bool neural;
	bool cartograph351Compatibility;
};

// Определяет возможности прибора. Модификация в допуске не участвует.
// Сигнатура 351 поддерживается отдельным совместимым исключением.
ToolCapabilities GetToolCapabilities(const ID& tool);
ToolCapabilities GetToolCapabilities(uint32_t signature);

bool IsSupportedTool(const ID& tool);

// Нейросетевой расчёт применим к приборам семейства LWD и картографам в режиме
// LWD, если сигнатура объявляет четыре передатчика.
bool IsNeuralLwd4Tx(const ID& tool);

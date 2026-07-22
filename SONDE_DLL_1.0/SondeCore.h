#pragma once
// SondeCore.h
// Базовые утилиты предметной области: классификация типа прибора по сигнатуре,
// проверка расширения файла, пересчёт показаний АЦП температуры.

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

// Единая политика определения возможностей прибора. Модификация не участвует
// в допуске. Сигнатура 351 сохраняется отдельным совместимым исключением.
ToolCapabilities GetToolCapabilities(const ID& tool);
ToolCapabilities GetToolCapabilities(uint32_t signature);
bool IsSupportedTool(const ID& tool);

// Нейросетевая модель применима к актуальной GP_DATA приборов семейства LWD
// и картографов в режиме LWD, если сигнатура объявляет четыре передатчика.
// Номер конкретного прибора и полная сигнатура на допуск не влияют.
bool IsNeuralLwd4Tx(const ID& tool);

float NormalizePhase(float phase);
int RxPhaseOrientationSign(uint32_t transmitterIndex, uint32_t rxPosition);

// Возвращает ненулевое значение, если строка str оканчивается на suffix.
int EndsWith(const char* str, const char* suffix);

// Пересчитывает значение АЦП в температуру в градусах Цельсия.
double temp_deg(int adc_value);

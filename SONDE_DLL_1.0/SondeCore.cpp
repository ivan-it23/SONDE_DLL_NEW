#include "stdafx.h"

#include <cstring>
#include <cmath>

#include "SondeCore.h"
#include "Constants.h"

// Разбор сигнатуры прибора на поля ID. Младшие 6 разрядов (signature % 1000000)
// кодируют прибор: type (3-значный код), type_ (семейство), N_Tx (число
// передатчиков), mod (модификация), number. Старшие разряды (signature / 1000000)
// задают размер структуры кадра (240/320); при нуле известным типам назначается 240.
ID get_sonde_id(uint32_t signature) {
	ID tool = {};
	tool.type = (signature % 1000000) / 1000;
	tool.type_ = (signature % 1000000) / 100000;
	tool.N_Tx = (signature % 100000) / 10000;
	tool.mod = (signature % 10000) / 1000;
	tool.number = (signature % 1000);
	if ((signature / 1000000) == 0) {
		// Старая прошивка не кодирует размер: канонический размер актуальных типов.
		if (tool.type == LWD_4Tx_NEW || tool.type == CARTOGRAPH_LWD_4Tx ||
			tool.type == AUTONOM_5Tx || tool.type == AUTONOM_5Tx_SDR || tool.type == LWD_3Tx) {
			tool.struct_size = 240;
		}
	}
	else {
		tool.struct_size = signature / 1000000;
	}
	return tool;
}

// Возможности прибора по его ID: поддержка расчёта, допуск к нейросети
// (N_Tx == 4 и семейство LWD/картограф) и число активных передатчиков.
ToolCapabilities GetToolCapabilities(const ID& tool) {
	ToolCapabilities result = {};
	result.identity = tool;
	result.cartograph351Compatibility = tool.type == CARTOGRAPH;

	const bool autonom = tool.type_ == 1 && (tool.N_Tx == 4 || tool.N_Tx == 5);
	const bool lwd = tool.type_ == 2 && (tool.N_Tx == 3 || tool.N_Tx == 4);
	const bool cartographLwd = tool.type_ == 3 && tool.N_Tx == 4;
	result.supported = autonom || lwd || cartographLwd || result.cartograph351Compatibility;
	result.neural = tool.N_Tx == 4 && (tool.type_ == 2 || tool.type_ == 3);
	result.activeTx = result.supported ? static_cast<uint8_t>(tool.N_Tx) : 0;
	return result;
}

ToolCapabilities GetToolCapabilities(uint32_t signature) {
	return GetToolCapabilities(get_sonde_id(signature));
}

bool IsSupportedTool(const ID& tool) {
	return GetToolCapabilities(tool).supported;
}

bool IsNeuralLwd4Tx(const ID& tool) {
	return GetToolCapabilities(tool).neural;
}

// Приведение фазы к диапазону (-PI, PI]. В текущем пути калибровки не вызывается.
float NormalizePhase(float phase) {
	while (phase > static_cast<float>(PI))
		phase -= static_cast<float>(2.0 * PI);
	while (phase <= static_cast<float>(-PI))
		phase += static_cast<float>(2.0 * PI);
	return phase;
}

// Знак ориентации фазы приёмника по номеру передатчика и Rx_Position.
// В текущем пути калибровки не вызывается (get_cal_signal чередует знак напрямую).
int RxPhaseOrientationSign(uint32_t transmitterIndex, uint32_t rxPosition) {
	const bool oddTransmitterNumber = ((transmitterIndex + 1U) & 1U) != 0;
	bool invert = !oddTransmitterNumber;
	if (rxPosition == 1U)
		invert = !invert;
	return invert ? -1 : 1;
}

int EndsWith(const char *str, const char *suffix) {
	if (!str || !suffix)
		return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// Перевод значения АЦП в температуру (градусы Цельсия).
double temp_deg(int adc_value) {
	double temp_deg;
	double v_in = 3300 * adc_value / pow(2, 10);
	if (v_in < 1500) temp_deg = (v_in - 500) / 10;
	else if (v_in >= 1500 && v_in < 1752.5) temp_deg = (v_in - 1500) / 10.1 + 100;
	else if (v_in >= 1752.5) temp_deg = (v_in - 1752.5) / 10.6 + 125;
	return temp_deg;
}

#include "stdafx.h"

#include <cstring>
#include <cmath>

#include "SondeCore.h"
#include "Constants.h"

ID get_sonde_id(uint32_t signature) {
	ID tool = {};
	const uint32_t buff = signature & 0x000FFFFFU;
	tool.type_ = buff / 100000;
	tool.N_Tx = (buff % 100000) / 10000;
	tool.mod = (buff % 10000) / 1000;
	tool.number = (buff % 1000);
	tool.type = buff / 1000;
	return tool;
}

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

float NormalizePhase(float phase) {
	while (phase > static_cast<float>(PI))
		phase -= static_cast<float>(2.0 * PI);
	while (phase <= static_cast<float>(-PI))
		phase += static_cast<float>(2.0 * PI);
	return phase;
}

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

// переводит температуру в градусы цельсия
double temp_deg(int adc_value) {
	double temp_deg;
	double v_in = 3300 * adc_value / pow(2, 10);
	if (v_in < 1500) temp_deg = (v_in - 500) / 10;
	else if (v_in >= 1500 && v_in < 1752.5) temp_deg = (v_in - 1500) / 10.1 + 100;
	else if (v_in >= 1752.5) temp_deg = (v_in - 1752.5) / 10.6 + 125;
	return temp_deg;
}

#include "stdafx.h"

#include <cstring>
#include <cmath>

#include "SondeCore.h"

ID get_sonde_id(uint32_t signature) {
	ID tool;
	uint32_t buff;
	buff = signature << 12;
	buff = buff >> 12;
	tool.type_ = buff / 100000;
	tool.N_Tx = (buff % 100000) / 10000;
	tool.mod = (buff % 10000) / 1000;
	tool.number = (buff % 1000);
	tool.type = buff / 1000;
	return tool;
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

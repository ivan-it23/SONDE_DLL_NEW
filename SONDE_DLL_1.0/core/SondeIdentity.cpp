#include "stdafx.h"

#include "SondeIdentity.h"
#include "Constants.h"

// Разбирает сигнатуру на поля ID. Младшие 6 разрядов кодируют прибор: type
// (3-значный код), type_ (семейство), N_Tx (число передатчиков), mod
// (модификация), number. Старшие разряды задают размер структуры кадра; если
// они нулевые, известным типам назначается 240 байт.
ID get_sonde_id(uint32_t signature) {
	ID tool = {};
	tool.type = (signature % 1000000) / 1000;
	tool.type_ = (signature % 1000000) / 100000;
	tool.N_Tx = (signature % 100000) / 10000;
	tool.mod = (signature % 10000) / 1000;
	tool.number = (signature % 1000);
	if ((signature / 1000000) == 0) {
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

// Определяет по ID, поддерживается ли прибор, допускается ли он к нейросетевому
// расчёту и сколько передатчиков считать активными.
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

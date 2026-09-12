#include "stdafx.h"

#include <fstream>
#include <cstring>
#include <sstream>

#include "Metrology.h"
#include "Constants.h"
#include "SondeIdentity.h"
#include "Logger.h"
#include "ErrorState.h"

using namespace std;

namespace {

bool has_extension(const char* path, const char* extension) {
	if (!path || !extension)
		return false;
	const char* dot = strrchr(path, '.');
	return dot != nullptr && _stricmp(dot, extension) == 0;
}

}

int validate_metrology(const GP_METROLOGY& metrology) {
	const ToolCapabilities capabilities = GetToolCapabilities(metrology.signature);
	if (!capabilities.supported) {
		std::ostringstream message;
		message << "Неподдерживаемый код типа прибора " << capabilities.identity.type
			<< " (семейство " << capabilities.identity.type_
			<< ", передатчиков " << capabilities.identity.N_Tx
			<< ", модификация " << capabilities.identity.mod << ").";
		SetSondeLastError(message.str());
		return err::kUnsupportedType;
	}

	if (capabilities.activeTx < 3 || capabilities.activeTx > config::kMaxTx) {
		SetSondeLastError("В сигнатуре метрологии указано недопустимое число передатчиков.");
		return err::kMetrologyLayout;
	}

	if (metrology.Rx_Position > 1U) {
		std::ostringstream message;
		message << "Недопустимое значение Rx_Position=" << metrology.Rx_Position << "; ожидается 0 или 1.";
		SetSondeLastError(message.str());
		return err::kMetrologyRxPosition;
	}

	std::ostringstream missing;
	bool hasMissingGeometry = false;
	for (uint32_t tx = 0; tx < capabilities.activeTx; ++tx) {
		if (metrology.L1[tx] == 0) {
			if (hasMissingGeometry) missing << ", ";
			missing << "L1[T" << (tx + 1) << "]=0";
			hasMissingGeometry = true;
		}
		if (metrology.L2[tx] == 0) {
			if (hasMissingGeometry) missing << ", ";
			missing << "L2[T" << (tx + 1) << "]=0";
			hasMissingGeometry = true;
		}
	}
	for (int freq = 0; freq < config::kFreqCount; ++freq) {
		if (metrology.F[freq] == 0) {
			if (hasMissingGeometry) missing << ", ";
			missing << "F[" << freq << "]=0";
			hasMissingGeometry = true;
		}
	}
	if (hasMissingGeometry) {
		std::ostringstream message;
		message << "Неполная геометрия в метрологии для прибора с "
			<< static_cast<int>(capabilities.activeTx) << " передатчиками: "
			<< missing.str() << ". Заполните L1 и L2 всех активных зондов и обе частоты.";
		SetSondeLastError(message.str());
		return err::kMetrologyGeometry;
	}

	return err::kOk;
}

int read_metrology_file(const char* path, GP_METROLOGY* outMetro, uint32_t* outSignature) {
	if (!path || !outMetro || !outSignature) {
		SetSondeLastError("Не заданы путь к файлу метрологии или выходные указатели.");
		return err::kInvalidArgument;
	}
	if (!has_extension(path, config::kMetrologyExtension)) {
		SetSondeLastError("Файл метрологии должен иметь расширение .bin.");
		if (debug == true) Test << "sonde_set: у файла метрологии не расширение .bin" << endl;
		return err::kMetrologyFile;
	}

	ifstream Metro;
	Metro.open(path, ios::binary);
	if (!Metro.is_open()) {
		SetSondeLastError(std::string("Не удалось открыть файл метрологии: ") + path);
		if (debug == true) Test << "sonde_set: не удалось открыть файл метрологии" << endl;
		return err::kMetrologyFile;
	}

	Metro.seekg(0, ios::end);
	const std::streamoff size = Metro.tellg();
	if (size != static_cast<std::streamoff>(sizeof(GP_METROLOGY))) {
		std::ostringstream message;
		message << "Недопустимый размер файла метрологии: " << size
			<< " байт; структура GP_METROLOGY требует ровно "
			<< sizeof(GP_METROLOGY) << " байт.";
		SetSondeLastError(message.str());
		return err::kMetrologySize;
	}

	GP_METROLOGY loaded = {};
	Metro.seekg(0, ios::beg);
	Metro.read(reinterpret_cast<char*>(&loaded), sizeof(loaded));
	if (!Metro || Metro.gcount() != static_cast<std::streamsize>(sizeof(loaded))) {
		SetSondeLastError("Файл метрологии оборван или прочитан не полностью.");
		return err::kMetrologyFile;
	}

	const int validation = validate_metrology(loaded);
	if (validation != err::kOk)
		return validation;

	*outMetro = loaded;
	*outSignature = loaded.signature;
	return err::kOk;
}

void fill_sonde_params(const GP_METROLOGY& metrology, SONDE_PARAM param[2][5], float Air[2][5], float Air_att_dB[2][5]) {
	std::memset(param, 0, sizeof(SONDE_PARAM) * config::kFreqCount * config::kMaxTx);
	std::memset(Air, 0, sizeof(float) * config::kFreqCount * config::kMaxTx);
	std::memset(Air_att_dB, 0, sizeof(float) * config::kFreqCount * config::kMaxTx);
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;
			param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;
			param[freq][Tx].f = float(metrology.F[freq]) * 1000;
			// фазовые нули воздуха переводятся из милиградусов в радианы
			Air[freq][Tx] = metrology.Air_ph[freq][Tx] / mG;
			// амплитудные нули воздуха хранятся в метрологии в децибелах
			Air_att_dB[freq][Tx] = metrology.Air_att_dB[freq][Tx];
		}
	}
}

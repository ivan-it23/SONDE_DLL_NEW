#include "stdafx.h"

#include <fstream>
#include <cstring>
#include <sstream>

#include "MetrologyLoader.h"
#include "Constants.h"
#include "SondeCore.h"
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

int validate_metrology(const GP_METROLOGY& metrology, ToolCapabilities* outCapabilities) {
	const ToolCapabilities capabilities = GetToolCapabilities(metrology.signature);
	if (!capabilities.supported) {
		std::ostringstream message;
		message << "Unsupported tool signature code " << capabilities.identity.type
			<< " (family=" << capabilities.identity.type_
			<< ", transmitters=" << capabilities.identity.N_Tx
			<< ", modification=" << capabilities.identity.mod << ").";
		SetSondeLastError(message.str());
		return err::kUnsupportedType;
	}

	if (capabilities.activeTx < 3 || capabilities.activeTx > config::kMaxTx) {
		SetSondeLastError("Metrology signature contains an invalid transmitter count.");
		return err::kMetrologyLayout;
	}

	if (metrology.Rx_Position > 1U) {
		std::ostringstream message;
		message << "Invalid Rx_Position=" << metrology.Rx_Position << "; expected 0 or 1.";
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
		message << "Metrology geometry is incomplete for a "
			<< static_cast<int>(capabilities.activeTx) << "-transmitter tool: "
			<< missing.str() << ". Fill every active L1/L2 value and both frequencies.";
		SetSondeLastError(message.str());
		return err::kMetrologyGeometry;
	}

	if (outCapabilities)
		*outCapabilities = capabilities;
	return err::kOk;
}

int read_metrology_file(const char* path, GP_METROLOGY* outMetro, uint32_t* outSignature) {
	if (!path || !outMetro || !outSignature) {
		SetSondeLastError("Metrology path and output pointers must not be null.");
		return err::kInvalidArgument;
	}
	if (!has_extension(path, config::kMetrologyExtension)) {
		SetSondeLastError("Metrology file must have the .bin extension.");
		if (debug == true) Test << "sonde_set Metrology file no .bin ext " << endl;
		return err::kMetrologyFile;
	}

	ifstream Metro;
	Metro.open(path, ios::binary);
	if (!Metro.is_open()) {
		SetSondeLastError(std::string("Unable to open metrology file: ") + path);
		if (debug == true) Test << "sonde_set Unable to open Metrology file  " << endl;
		return err::kMetrologyFile;
	}

	Metro.seekg(0, ios::end);
	const std::streamoff size = Metro.tellg();
	if (size != static_cast<std::streamoff>(sizeof(GP_METROLOGY))) {
		std::ostringstream message;
		message << "Invalid metrology file size: " << size
			<< " bytes; the current GP_METROLOGY layout requires exactly "
			<< sizeof(GP_METROLOGY) << " bytes.";
		SetSondeLastError(message.str());
		return err::kMetrologySize;
	}

	GP_METROLOGY loaded = {};
	Metro.seekg(0, ios::beg);
	Metro.read(reinterpret_cast<char*>(&loaded), sizeof(loaded));
	if (!Metro || Metro.gcount() != static_cast<std::streamsize>(sizeof(loaded))) {
		SetSondeLastError("Metrology file is truncated or could not be read completely.");
		return err::kMetrologyFile;
	}

	const int validation = validate_metrology(loaded, nullptr);
	if (validation != err::kOk)
		return validation;

	*outMetro = loaded;
	*outSignature = loaded.signature;
	return err::kOk;
}

void fill_sonde_params(const GP_METROLOGY& metrologyIn, SONDE_PARAM param[2][5], float Air[2][5]) {
	GP_METROLOGY metrology = metrologyIn;
	const ToolCapabilities capabilities = GetToolCapabilities(metrology.signature);
	if (metrology.D_sonde_mm == 0) {
		const ID tool = get_sonde_id(metrology.signature);
		metrology.D_sonde_mm = (tool.type_ == 2 || (tool.type_ == 3 && tool.N_Tx == 4))
			? config::kDefaultLwdSondeDiameterMm
			: config::kDefaultAutonomSondeDiameterMm;
	}

	std::memset(param, 0, sizeof(SONDE_PARAM) * config::kFreqCount * config::kMaxTx);
	std::memset(Air, 0, sizeof(float) * config::kFreqCount * config::kMaxTx);
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < capabilities.activeTx; Tx++) {
			param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;
			param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;
			param[freq][Tx].f = float(metrology.F[freq]) * 1000;
			param[freq][Tx].D_sonde_m = float(metrology.D_sonde_mm) / 1000;
			Air[freq][Tx] = metrology.Air_zz[freq][Tx] / config::kFirmwareMilligradPerRadian;
		}
	}
}

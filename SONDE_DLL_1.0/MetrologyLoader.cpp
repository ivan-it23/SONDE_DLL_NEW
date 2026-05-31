#include "stdafx.h"

#include <fstream>

#include "MetrologyLoader.h"
#include "Constants.h"
#include "SondeCore.h"
#include "Logger.h"

using namespace std;

int read_metrology_file(const char* path, GP_METROLOGY* outMetro, uint32_t* outSignature) {
	if (!EndsWith(path, config::kMetrologyExtension)) {
		if (debug == true) Test << "sonde_set Metrology file no .bin ext " << endl;
		return err::kMetrologyFile;
	}

	ifstream Metro;
	Metro.open(path, ios::binary);
	if (!Metro.is_open()) {
		if (debug == true) Test << "sonde_set Unable to open Metrology file  " << endl;
		return err::kMetrologyFile;
	}

	uint32_t signature;
	Metro.read((char*)&signature, sizeof(uint32_t));
	Metro.seekg(0, ios::beg);
	*outSignature = signature;

	Metro.read((char*)outMetro, sizeof(GP_METROLOGY));
	Metro.close();

	return err::kOk;
}

void fill_sonde_params(const GP_METROLOGY& metrologyIn, SONDE_PARAM param[2][5], float Air[2][5]) {
	GP_METROLOGY metrology = metrologyIn;
	if (metrology.D_sonde_mm == 0)
		metrology.D_sonde_mm = config::kDefaultSondeDiameterMm; // autonomy 1DDS

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;
			param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;
			param[freq][Tx].f = float(metrology.F[freq]) * 1000;
			param[freq][Tx].D_sonde_m = float(metrology.D_sonde_mm) / 1000;
			Air[freq][Tx] = metrology.Air_zz[freq][Tx] / mG;
		}
	}
}

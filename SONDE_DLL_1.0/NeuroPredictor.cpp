#include "stdafx.h"

#include <windows.h>
#include <string>

#include "NeuroPredictor.h"
#include "Constants.h"
#include "Logger.h"
#include "neuro_api.h"

using namespace std;

namespace {

HMODULE hNeuroDll = NULL;
void* hNeuroPredictor = NULL;
int activeToolType = 0;
PFN_GeoPredictor_Create fnGeoPredictor_Create = nullptr;
PFN_GeoPredictor_Predict fnGeoPredictor_Predict = nullptr;
PFN_GeoPredictor_Destroy fnGeoPredictor_Destroy = nullptr;
PFN_GeoPredictor_GetLastError fnGeoPredictor_GetLastError = nullptr;

// Каталог исполняемого модуля процесса (для поиска NEURO_TEST.dll и весов).
std::string GetDllDirectory() {
	char path[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(NULL, path, MAX_PATH) > 0) {
		std::string s(path);
		size_t pos = s.find_last_of("\\/");
		if (pos != std::string::npos) return s.substr(0, pos + 1);
	}
	return std::string();
}

bool FileExists(const std::string& path) {
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES &&
		(attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

const char* ToolWeightsSubdir(int toolType) {
	switch (toolType) {
	case CARTOGRAPH_LWD_4Tx: return "CARTOGRAPH_LWD_4Tx-359";
	case LWD_4Tx_NEW: return "LWD_4Tx_NEW-242";
	case LWD_4Tx: return "LWD_4Tx-241";
	case CARTOGRAPH: return "CARTOGRAPH-351";
	case AUTONOM_4Tx: return "AUTONOM_4Tx-141";
	case AUTONOM_5Tx: return "AUTONOM_5Tx-151";
	case AUTONOM_5Tx_SDR: return "AUTONOM_5Tx_SDR-152";
	case LWD_3Tx: return "LWD_3Tx-231";
	default: return nullptr;
	}
}

std::string JoinPath(const std::string& left, const std::string& right) {
	if (left.empty()) return right;
	const char last = left[left.size() - 1];
	if (last == '\\' || last == '/') return left + right;
	return left + "\\" + right;
}

std::string BuildWeightsDir(const std::string& baseDir, int toolType) {
	const char* subdir = ToolWeightsSubdir(toolType);
	if (subdir == nullptr) return std::string();
	return JoinPath(JoinPath(baseDir, config::kNeuroWeightsRootDir), subdir);
}

bool WeightsComplete(const std::string& weightsDir) {
	const char* files[] = {
		"w1.bin", "w2.bin", "w3.bin", "w4.bin",
		"b1.bin", "b2.bin", "b3.bin", "b4.bin",
		"in_mean.bin", "in_scale.bin", "out_min.bin", "out_scale.bin"
	};
	for (int i = 0; i < 12; i++) {
		if (!FileExists(JoinPath(weightsDir, files[i]))) return false;
	}
	return true;
}

void ReleaseNeuro() {
	if (hNeuroPredictor != NULL && fnGeoPredictor_Destroy != nullptr) {
		fnGeoPredictor_Destroy(hNeuroPredictor);
	}
	hNeuroPredictor = NULL;
	if (hNeuroDll != NULL) {
		FreeLibrary(hNeuroDll);
	}
	hNeuroDll = NULL;
	fnGeoPredictor_Create = nullptr;
	fnGeoPredictor_Predict = nullptr;
	fnGeoPredictor_Destroy = nullptr;
	fnGeoPredictor_GetLastError = nullptr;
	activeToolType = 0;
}

} // namespace

int neuro_init(int toolType) {
	if (hNeuroDll != NULL && hNeuroPredictor != NULL && activeToolType == toolType)
		return err::kOk; // уже инициализирован

	if (hNeuroDll != NULL || hNeuroPredictor != NULL)
		ReleaseNeuro();

	std::string dllDir = GetDllDirectory();
	std::string neuroPath = dllDir + config::kNeuroDllName;
	hNeuroDll = LoadLibraryA(neuroPath.c_str());
	if (hNeuroDll == NULL) {
		// запасной вариант: поиск по стандартным путям загрузчика
		hNeuroDll = LoadLibraryA(config::kNeuroDllName);
	}
	if (hNeuroDll == NULL) {
		if (debug == true) Test << "sonde_set unable to load NEURO_TEST.dll" << endl;
		return err::kNeuroDllNotLoaded;
	}

	fnGeoPredictor_Create = (PFN_GeoPredictor_Create)GetProcAddress(hNeuroDll, config::kNeuroCreateFn);
	fnGeoPredictor_Predict = (PFN_GeoPredictor_Predict)GetProcAddress(hNeuroDll, config::kNeuroPredictFn);
	fnGeoPredictor_Destroy = (PFN_GeoPredictor_Destroy)GetProcAddress(hNeuroDll, config::kNeuroDestroyFn);
	fnGeoPredictor_GetLastError = (PFN_GeoPredictor_GetLastError)GetProcAddress(hNeuroDll, config::kNeuroLastErrorFn);
	if (!fnGeoPredictor_Create || !fnGeoPredictor_Predict || !fnGeoPredictor_Destroy) {
		if (debug == true) Test << "sonde_set unable to get neuro functions" << endl;
		ReleaseNeuro();
		return err::kNeuroFuncNotFound;
	}

	std::string weightsDir = BuildWeightsDir(dllDir, toolType);
	if (weightsDir.empty() || !WeightsComplete(weightsDir)) {
		if (debug == true) {
			Test << "sonde_set no neural weights for tool type " << toolType
			     << ", weights dir: " << weightsDir << endl;
		}
		ReleaseNeuro();
		return err::kNeuroWeightsNotFound;
	}
	hNeuroPredictor = fnGeoPredictor_Create(weightsDir.c_str());
	if (hNeuroPredictor == NULL) {
		if (debug == true) {
			Test << "sonde_set unable to create neuro predictor, weights dir: " << weightsDir << endl;
			if (fnGeoPredictor_GetLastError)
				Test << "neuro error: " << fnGeoPredictor_GetLastError() << endl;
		}
		ReleaseNeuro();
		return err::kNeuroCreateFailed;
	}
	activeToolType = toolType;

	if (debug == true) {
		Test << "neuro_init: DLL loaded from " << neuroPath << endl;
		Test << "neuro_init: weights dir " << weightsDir << endl;
		Test << "neuro_init: predictor created OK" << endl;
	}
	return err::kOk;
}

bool neuro_available() {
	return (hNeuroPredictor != NULL && fnGeoPredictor_Predict != NULL);
}

int neuro_predict(const float* inputs, float* outputs) {
	return fnGeoPredictor_Predict(hNeuroPredictor, inputs, outputs);
}

const char* neuro_last_error() {
	if (fnGeoPredictor_GetLastError)
		return fnGeoPredictor_GetLastError();
	return nullptr;
}

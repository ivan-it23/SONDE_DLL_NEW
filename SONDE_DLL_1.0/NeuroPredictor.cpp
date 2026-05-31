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

} // namespace

int neuro_init() {
	if (hNeuroDll != NULL)
		return err::kOk; // уже инициализирован

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
		FreeLibrary(hNeuroDll); hNeuroDll = NULL;
		return err::kNeuroFuncNotFound;
	}

	std::string weightsDir = dllDir + config::kNeuroWeightsDir;
	hNeuroPredictor = fnGeoPredictor_Create(weightsDir.c_str());
	if (hNeuroPredictor == NULL) {
		// запасной вариант: относительный путь к весам
		weightsDir = config::kNeuroWeightsDir;
		hNeuroPredictor = fnGeoPredictor_Create(weightsDir.c_str());
	}
	if (hNeuroPredictor == NULL) {
		if (debug == true) {
			Test << "sonde_set unable to create neuro predictor, weights dir: " << weightsDir << endl;
			if (fnGeoPredictor_GetLastError)
				Test << "neuro error: " << fnGeoPredictor_GetLastError() << endl;
		}
		FreeLibrary(hNeuroDll); hNeuroDll = NULL;
		return err::kNeuroCreateFailed;
	}

	if (debug == true) Test << "sonde_set neuro predictor created OK" << endl;
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

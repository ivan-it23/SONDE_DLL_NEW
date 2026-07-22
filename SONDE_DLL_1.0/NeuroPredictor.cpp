#include "stdafx.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "NeuroPredictor.h"
#include "Constants.h"
#include "Logger.h"
#include "SondeCore.h"
#include "ErrorState.h"
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

extern "C" IMAGE_DOS_HEADER __ImageBase;

// Каталог именно SONDE_DLL_1.0.dll, а не EXE вызывающего приложения.
std::string GetDllDirectory() {
	char path[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(reinterpret_cast<HMODULE>(&__ImageBase), path, MAX_PATH) > 0) {
		std::string s(path);
		size_t pos = s.find_last_of("\\/");
		if (pos != std::string::npos) return s.substr(0, pos + 1);
	}
	return std::string();
}

bool DirectoryExists(const std::string& path) {
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES &&
		(attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool FileExists(const std::string& path) {
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES &&
		(attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::string JoinPath(const std::string& left, const std::string& right) {
	if (left.empty()) return right;
	const char last = left[left.size() - 1];
	if (last == '\\' || last == '/') return left + right;
	return left + "\\" + right;
}

bool WeightsComplete(const std::string& weightsDir, std::string* missingFile) {
	const char* files[] = {
		"w1.bin", "w2.bin", "w3.bin", "w4.bin",
		"b1.bin", "b2.bin", "b3.bin", "b4.bin",
		"in_mean.bin", "in_scale.bin", "out_min.bin", "out_scale.bin"
	};
	for (int i = 0; i < 12; i++) {
		if (!FileExists(JoinPath(weightsDir, files[i]))) {
			if (missingFile) *missingFile = files[i];
			return false;
		}
	}
	return true;
}

std::string BuildWeightsDir(const std::string& baseDir, int toolType, std::string* searchDescription) {
	std::ostringstream suffix;
	suffix << '-' << std::setw(3) << std::setfill('0') << toolType;
	const std::string root = JoinPath(baseDir, config::kNeuroWeightsRootDir);
	const std::string pattern = JoinPath(root, "*" + suffix.str());
	if (searchDescription) *searchDescription = pattern;
	if (!DirectoryExists(root))
		return std::string();

	WIN32_FIND_DATAA entry = {};
	HANDLE find = FindFirstFileA(pattern.c_str(), &entry);
	if (find == INVALID_HANDLE_VALUE)
		return std::string();

	std::string firstIncomplete;
	do {
		if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			continue;
		const std::string candidate = JoinPath(root, entry.cFileName);
		std::string missing;
		if (WeightsComplete(candidate, &missing)) {
			FindClose(find);
			return candidate;
		}
		if (firstIncomplete.empty())
			firstIncomplete = candidate + " (missing " + missing + ")";
	} while (FindNextFileA(find, &entry));
	FindClose(find);

	if (searchDescription && !firstIncomplete.empty())
		*searchDescription += "; incomplete candidate: " + firstIncomplete;
	return std::string();
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

	std::string dllDir = GetDllDirectory();
	if (dllDir.empty()) {
		SetSondeLastError("Unable to determine the SONDE DLL directory for neural dependencies.");
		return err::kNeuroDllNotLoaded;
	}
	std::string neuroPath = dllDir + config::kNeuroDllName;
	HMODULE candidateDll = LoadLibraryExA(
		neuroPath.c_str(), NULL,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (candidateDll == NULL) {
		std::ostringstream message;
		message << "Unable to load neural runtime '" << neuroPath
			<< "' (Win32 error " << GetLastError() << ").";
		SetSondeLastError(message.str());
		if (debug == true) Test << "sonde_set unable to load NEURO_TEST.dll" << endl;
		return err::kNeuroDllNotLoaded;
	}

	PFN_GeoPredictor_Create candidateCreate =
		(PFN_GeoPredictor_Create)GetProcAddress(candidateDll, config::kNeuroCreateFn);
	PFN_GeoPredictor_Predict candidatePredict =
		(PFN_GeoPredictor_Predict)GetProcAddress(candidateDll, config::kNeuroPredictFn);
	PFN_GeoPredictor_Destroy candidateDestroy =
		(PFN_GeoPredictor_Destroy)GetProcAddress(candidateDll, config::kNeuroDestroyFn);
	PFN_GeoPredictor_GetLastError candidateLastError =
		(PFN_GeoPredictor_GetLastError)GetProcAddress(candidateDll, config::kNeuroLastErrorFn);
	if (!candidateCreate || !candidatePredict || !candidateDestroy) {
		SetSondeLastError("NEURO_TEST.dll does not export the required predictor functions.");
		if (debug == true) Test << "sonde_set unable to get neuro functions" << endl;
		FreeLibrary(candidateDll);
		return err::kNeuroFuncNotFound;
	}

	std::string searchDescription;
	std::string weightsDir = BuildWeightsDir(dllDir, toolType, &searchDescription);
	if (weightsDir.empty()) {
		std::ostringstream message;
		message << "Neural weights for signature " << toolType
			<< " were not found. Expected a complete directory ending with '-"
			<< std::setw(3) << std::setfill('0') << toolType
			<< "' next to the SONDE DLL. Search: " << searchDescription << ".";
		SetSondeLastError(message.str());
		if (debug == true) {
			Test << "sonde_set no neural weights for tool type " << toolType
			     << ", search: " << searchDescription << endl;
		}
		FreeLibrary(candidateDll);
		return err::kNeuroWeightsNotFound;
	}
	void* candidatePredictor = candidateCreate(weightsDir.c_str());
	if (candidatePredictor == NULL) {
		std::string detail = "Unable to create neural predictor from weights directory '" + weightsDir + "'.";
		if (candidateLastError && candidateLastError())
			detail += std::string(" Neural runtime: ") + candidateLastError();
		SetSondeLastError(detail);
		if (debug == true) {
			Test << "sonde_set unable to create neuro predictor, weights dir: " << weightsDir << endl;
			if (candidateLastError)
				Test << "neuro error: " << candidateLastError() << endl;
		}
		FreeLibrary(candidateDll);
		return err::kNeuroCreateFailed;
	}

	// Старый предиктор остаётся рабочим до полного успеха создания нового.
	ReleaseNeuro();
	hNeuroDll = candidateDll;
	hNeuroPredictor = candidatePredictor;
	fnGeoPredictor_Create = candidateCreate;
	fnGeoPredictor_Predict = candidatePredict;
	fnGeoPredictor_Destroy = candidateDestroy;
	fnGeoPredictor_GetLastError = candidateLastError;
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
	if (!inputs || !outputs) {
		SetSondeLastError("Neural predictor input and output pointers must not be null.");
		return err::kInvalidArgument;
	}
	if (!neuro_available()) {
		SetSondeLastError("Neural predictor is not initialized for the current metrology.");
		return err::kNeuroNotInitialized;
	}
	return fnGeoPredictor_Predict(hNeuroPredictor, inputs, outputs);
}

const char* neuro_last_error() {
	if (fnGeoPredictor_GetLastError)
		return fnGeoPredictor_GetLastError();
	return nullptr;
}

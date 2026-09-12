#include "stdafx.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "NeuroPredictor.h"
#include "Constants.h"
#include "Logger.h"
#include "SondeIdentity.h"
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

// Извлекает завершающий 3-значный код прибора из имени папки весов
// (часть после последнего '-'). Возвращает -1, если формат не распознан.
int ParseTrailingToolCode(const std::string& name) {
	size_t dash = name.find_last_of('-');
	if (dash == std::string::npos)
		return -1;
	const std::string tail = name.substr(dash + 1);
	if (tail.size() != 3)
		return -1;
	for (size_t i = 0; i < tail.size(); ++i) {
		if (tail[i] < '0' || tail[i] > '9')
			return -1;
	}
	return (tail[0] - '0') * 100 + (tail[1] - '0') * 10 + (tail[2] - '0');
}

// Выбор каталога весов. Модификация прибора (3-я цифра кода) НЕ влияет на выбор:
// обязателен матч по типу прибора и числу передатчиков (первые 2 цифры кода).
// Точное совпадение по модификации предпочтительно; при его отсутствии берётся
// любой полный каталог с тем же типом+числом передатчиков (fallback), чтобы
// модификация не блокировала загрузку весов.
std::string BuildWeightsDir(const std::string& baseDir, int toolType, std::string* searchDescription) {
	const int prefix = toolType / 10;   // тип прибора + число передатчиков, напр. 24, 34
	const std::string root = JoinPath(baseDir, config::kNeuroWeightsRootDir);
	if (searchDescription) {
		std::ostringstream desc;
		desc << JoinPath(root, "*-") << std::setw(2) << std::setfill('0') << prefix
		     << "? (тип+число передатчиков " << prefix << ", модификация не учитывается)";
		*searchDescription = desc.str();
	}
	if (!DirectoryExists(root))
		return std::string();

	WIN32_FIND_DATAA entry = {};
	HANDLE find = FindFirstFileA(JoinPath(root, "*").c_str(), &entry);
	if (find == INVALID_HANDLE_VALUE)
		return std::string();

	std::string exactMatch;      // совпали все 3 цифры (предпочтительный вариант)
	std::string prefixMatch;     // совпали только тип + число передатчиков (fallback)
	std::string firstIncomplete;
	do {
		if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			continue;
		const std::string name = entry.cFileName;
		const int code = ParseTrailingToolCode(name);
		if (code < 0 || code / 10 != prefix)
			continue;            // тип прибора или число передатчиков не совпали
		const std::string candidate = JoinPath(root, name);
		std::string missing;
		if (!WeightsComplete(candidate, &missing)) {
			if (firstIncomplete.empty())
				firstIncomplete = candidate + " (отсутствует " + missing + ")";
			continue;
		}
		if (code == toolType) {
			exactMatch = candidate;   // точная модификация — приоритетный выбор, поиск прекращается
			break;
		}
		if (prefixMatch.empty())
			prefixMatch = candidate;  // первый полный каталог с тем же типом+N_Tx
	} while (FindNextFileA(find, &entry));
	FindClose(find);

	if (!exactMatch.empty())
		return exactMatch;
	if (!prefixMatch.empty())
		return prefixMatch;

	if (searchDescription && !firstIncomplete.empty())
		*searchDescription += "; неполный каталог: " + firstIncomplete;
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

// Загрузка NEURO_TEST.dll, привязка экспортов и создание предиктора из каталога
// весов для типа прибора. Повторный вызов с тем же типом ничего не делает.
int neuro_init(int toolType) {
	if (hNeuroDll != NULL && hNeuroPredictor != NULL && activeToolType == toolType)
		return err::kOk; // уже инициализирован

	std::string dllDir = GetDllDirectory();
	if (dllDir.empty()) {
		SetSondeLastError("Не удалось определить каталог библиотеки SONDE_DLL_1.0.dll для поиска нейросетевых зависимостей.");
		return err::kNeuroDllNotLoaded;
	}
	std::string neuroPath = dllDir + config::kNeuroDllName;
	HMODULE candidateDll = LoadLibraryExA(
		neuroPath.c_str(), NULL,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (candidateDll == NULL) {
		std::ostringstream message;
		message << "Не удалось загрузить нейросетевую библиотеку '" << neuroPath
			<< "' (ошибка Win32 " << GetLastError() << ").";
		SetSondeLastError(message.str());
		if (debug == true) Test << "sonde_set: не удалось загрузить NEURO_TEST.dll" << endl;
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
		SetSondeLastError("В NEURO_TEST.dll не найдены требуемые функции предиктора.");
		if (debug == true) Test << "sonde_set: не найдены функции нейросети" << endl;
		FreeLibrary(candidateDll);
		return err::kNeuroFuncNotFound;
	}

	std::string searchDescription;
	std::string weightsDir = BuildWeightsDir(dllDir, toolType, &searchDescription);
	if (weightsDir.empty()) {
		std::ostringstream message;
		message << "Не найдены веса нейросети для типа прибора " << toolType
			<< ". Рядом с SONDE_DLL_1.0.dll должен находиться полный каталог, имя которого завершается кодом с тем же типом прибора и числом передатчиков (первые две цифры '"
			<< (toolType / 10)
			<< "'; модификация не учитывается). Поиск: " << searchDescription << ".";
		SetSondeLastError(message.str());
		if (debug == true) {
			Test << "sonde_set: нет весов нейросети для типа прибора " << toolType
			     << ", поиск: " << searchDescription << endl;
		}
		FreeLibrary(candidateDll);
		return err::kNeuroWeightsNotFound;
	}
	void* candidatePredictor = candidateCreate(weightsDir.c_str());
	if (candidatePredictor == NULL) {
		std::string detail = "Не удалось создать нейросетевой предиктор из каталога весов '" + weightsDir + "'.";
		if (candidateLastError && candidateLastError())
			detail += std::string(" Сообщение нейросетевой библиотеки: ") + candidateLastError();
		SetSondeLastError(detail);
		if (debug == true) {
			Test << "sonde_set: не удалось создать нейросетевой предиктор, каталог весов: " << weightsDir << endl;
			if (candidateLastError)
				Test << "ошибка нейросети: " << candidateLastError() << endl;
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
		Test << "neuro_init: библиотека загружена из " << neuroPath << endl;
		Test << "neuro_init: каталог весов " << weightsDir << endl;
		Test << "neuro_init: предиктор создан" << endl;
	}
	return err::kOk;
}

// Готовность предиктора к предсказанию.
bool neuro_available() {
	return (hNeuroPredictor != NULL && fnGeoPredictor_Predict != NULL);
}

// Предсказание нейросети через GeoPredictor_Predict: массив входов -> выходов.
int neuro_predict(const float* inputs, float* outputs) {
	if (!inputs || !outputs) {
		SetSondeLastError("Не заданы входной или выходной массив нейросетевого предиктора.");
		return err::kInvalidArgument;
	}
	if (!neuro_available()) {
		SetSondeLastError("Нейросетевой предиктор не инициализирован для загруженной метрологии.");
		return err::kNeuroNotInitialized;
	}
	return fnGeoPredictor_Predict(hNeuroPredictor, inputs, outputs);
}

const char* neuro_last_error() {
	if (fnGeoPredictor_GetLastError)
		return fnGeoPredictor_GetLastError();
	return nullptr;
}

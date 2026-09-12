#include "stdafx.h"

#include <cstring>
#include <fstream>
#include <limits>

#include "DataFile.h"
#include "Constants.h"
#include "Logger.h"
#include "SondeState.h"
#include "SondeIdentity.h"
#include "ErrorState.h"

#include <cmath>
#include <sstream>

using namespace std;

namespace {

constexpr int kDevFrameHeaderSize = 11;

bool has_extension(const char* path, const char* extension) {
	if (!path || !extension)
		return false;

	const char* dot = strrchr(path, '.');
	return dot != nullptr && _stricmp(dot, extension) == 0;
}

} // namespace

int scan_data_file(
	const char* dataPath,
	uint32_t* frameCount,
	int* frameHeaderSize,
	uint32_t* dataSignature) {
	if (!dataPath || !frameCount || !frameHeaderSize || !dataSignature) {
		SetSondeLastError("get_data_file_info: не заданы путь к файлу данных или один из трёх выходных указателей.");
		return err::kInvalidArgument;
	}

	*frameCount = 0;
	*frameHeaderSize = 0;
	*dataSignature = 0;

	if (!sonde_initialized || global_signature == 0) {
		SetSondeLastError("Перед проверкой файла данных необходимо успешно выполнить sonde_set.");
		return err::kMetrologyNotInitialized;
	}

	int headerSize = 0;
	if (has_extension(dataPath, ".dev")) {
		headerSize = kDevFrameHeaderSize;
	}
	else if (!has_extension(dataPath, ".bin")) {
		SetSondeLastError("Неподдерживаемое расширение файла данных: ожидается .DEV или .bin.");
		if (debug == true) Test << "get_data_file_info: неподдерживаемое расширение файла данных: " << dataPath << endl;
		return err::kDataFileExtension;
	}

	ifstream data(dataPath, ios::binary);
	if (!data.is_open()) {
		SetSondeLastError(std::string("Не удалось открыть файл данных: ") + dataPath);
		if (debug == true) Test << "get_data_file_info: не удалось открыть файл данных: " << dataPath << endl;
		return err::kDataFile;
	}

	data.seekg(0, ios::end);
	const streamoff fileSize = data.tellg();

	// Размер кадра прибора определяется по struct_size из сигнатуры первого
	// кадра: старая прошивка -> 240 байт, новая (с амплитудным каналом) -> 320.
	if (fileSize < static_cast<streamoff>(headerSize + sizeof(uint32_t))) {
		SetSondeLastError("Файл данных меньше одного кадра.");
		return err::kDataFileLayout;
	}
	uint32_t probeSignature = 0;
	data.seekg(headerSize, ios::beg);
	data.read(reinterpret_cast<char*>(&probeSignature), sizeof(probeSignature));
	if (!data) {
		SetSondeLastError("Не удалось прочитать сигнатуру из файла данных.");
		return err::kDataFileLayout;
	}
	const ID probeTool = get_sonde_id(probeSignature);
	streamoff frameDataSize = static_cast<streamoff>(sizeof(GP_DATA));
	if (probeTool.struct_size > 0 && probeTool.struct_size <= sizeof(GP_DATA))
		frameDataSize = static_cast<streamoff>(probeTool.struct_size);
	const streamoff recordSize = frameDataSize + headerSize;
	if (fileSize <= 0 || recordSize <= 0 || fileSize % recordSize != 0) {
		std::ostringstream message;
		message << "Недопустимый размер файла данных: " << fileSize << " байт при размере записи "
			<< recordSize << " байт (служебная часть " << headerSize
			<< ", кадр " << frameDataSize << ").";
		SetSondeLastError(message.str());
		if (debug == true) {
			Test << "get_data_file_info: недопустимая раскладка файла, размер=" << fileSize
				 << " размер записи=" << recordSize << endl;
		}
		return err::kDataFileLayout;
	}

	const unsigned long long count = static_cast<unsigned long long>(fileSize / recordSize);
	if (count == 0 || count > (std::numeric_limits<uint32_t>::max)()) {
		SetSondeLastError("Недопустимое число кадров в файле данных.");
		return err::kDataFileLayout;
	}

	uint32_t firstSignature = 0;
	for (unsigned long long frame = 0; frame < count; ++frame) {
		const streamoff payloadOffset = static_cast<streamoff>(frame) * recordSize + headerSize;
		data.seekg(payloadOffset, ios::beg);

		// Читаем ровно frameDataSize байт в обнулённую GP_DATA (усечение по struct_size).
		GP_DATA current = {};
		data.read(reinterpret_cast<char*>(&current), frameDataSize);
		if (!data) {
			SetSondeLastError("Файл данных оборван при чтении кадра GP_DATA.");
			if (debug == true) Test << "get_data_file_info: не удалось прочитать кадр " << frame << endl;
			return err::kDataFileLayout;
		}

		if (frame == 0)
			firstSignature = current.signature;

		// Сравниваем только идентификатор прибора (младшие 6 разрядов).
		if ((current.signature % 1000000u) != (firstSignature % 1000000u) ||
			(current.signature % 1000000u) != (global_signature % 1000000u)) {
			std::ostringstream message;
			message << "Несовпадение сигнатур метрологии и данных на кадре " << frame
				<< ": метрология=" << global_signature
				<< ", данные=" << current.signature << ".";
			SetSondeLastError(message.str());
			if (debug == true) {
				Test << "get_data_file_info: несовпадение сигнатур на кадре " << frame
					 << ": метрология=" << global_signature
					 << " данные=" << current.signature << endl;
			}
			return err::kFrameSignatureMismatch;
		}

		for (int freq = 0; freq < config::kFreqCount; ++freq) {
			for (uint32_t tx = 0; tx < global_active_tx; ++tx) {
				if (!std::isfinite(current.rho_smt[freq][tx]) ||
					!std::isfinite(current.phase_smt[freq][tx]) ||
					!std::isfinite(current.DELTA_PH[freq][tx])) {
					std::ostringstream message;
					message << "Кадр данных " << frame << " содержит нечисловое значение: F"
						<< freq << " T" << (tx + 1) << ".";
					SetSondeLastError(message.str());
					return err::kDataFileLayout;
				}
			}
		}
	}

	*frameCount = static_cast<uint32_t>(count);
	*frameHeaderSize = headerSize;
	*dataSignature = firstSignature;
	return err::kOk;
}

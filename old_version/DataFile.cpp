#include "stdafx.h"

#include <cstring>
#include <fstream>
#include <limits>

#include "DataFile.h"
#include "Constants.h"
#include "Logger.h"
#include "SondeState.h"
#include "SondeCore.h"
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

extern "C" __declspec(dllexport) int get_data_file_info(
	const char* dataPath,
	uint32_t* frameCount,
	int* frameHeaderSize,
	uint32_t* dataSignature) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!dataPath || !frameCount || !frameHeaderSize || !dataSignature) {
		SetSondeLastError("get_data_file_info requires a path and three non-null output pointers.");
		return err::kInvalidArgument;
	}

	*frameCount = 0;
	*frameHeaderSize = 0;
	*dataSignature = 0;

	if (!sonde_initialized || global_signature == 0) {
		SetSondeLastError("sonde_set must complete successfully before validating a data file.");
		return err::kMetrologyNotInitialized;
	}

	int headerSize = 0;
	if (has_extension(dataPath, ".dev")) {
		headerSize = kDevFrameHeaderSize;
	}
	else if (!has_extension(dataPath, ".bin")) {
		SetSondeLastError("Unsupported data file extension; expected .DEV or .bin.");
		if (debug == true) Test << "get_data_file_info unsupported data extension: " << dataPath << endl;
		return err::kDataFileExtension;
	}

	ifstream data(dataPath, ios::binary);
	if (!data.is_open()) {
		SetSondeLastError(std::string("Unable to open data file: ") + dataPath);
		if (debug == true) Test << "get_data_file_info unable to open data file: " << dataPath << endl;
		return err::kDataFile;
	}

	data.seekg(0, ios::end);
	const streamoff fileSize = data.tellg();
	const streamoff recordSize = static_cast<streamoff>(sizeof(GP_DATA) + headerSize);
	if (fileSize <= 0 || recordSize <= 0 || fileSize % recordSize != 0) {
		std::ostringstream message;
		message << "Invalid data file size " << fileSize << " bytes for record size "
			<< recordSize << " bytes (header=" << headerSize
			<< ", GP_DATA=" << sizeof(GP_DATA) << ").";
		SetSondeLastError(message.str());
		if (debug == true) {
			Test << "get_data_file_info invalid data layout: size=" << fileSize
				 << " record_size=" << recordSize << endl;
		}
		return err::kDataFileLayout;
	}

	const unsigned long long count = static_cast<unsigned long long>(fileSize / recordSize);
	if (count == 0 || count > (std::numeric_limits<uint32_t>::max)()) {
		SetSondeLastError("Data file contains an unsupported number of frames.");
		return err::kDataFileLayout;
	}

	uint32_t firstSignature = 0;
	for (unsigned long long frame = 0; frame < count; ++frame) {
		const streamoff payloadOffset = static_cast<streamoff>(frame) * recordSize + headerSize;
		data.seekg(payloadOffset, ios::beg);

		GP_DATA current = {};
		data.read(reinterpret_cast<char*>(&current), sizeof(current));
		if (!data) {
			SetSondeLastError("Data file is truncated while reading a GP_DATA frame.");
			if (debug == true) Test << "get_data_file_info unable to read frame " << frame << endl;
			return err::kDataFileLayout;
		}

		if (frame == 0)
			firstSignature = current.signature;

		if (current.signature != firstSignature || current.signature != global_signature) {
			std::ostringstream message;
			message << "Metrology/data signature mismatch at frame " << frame
				<< ": metrology=" << global_signature
				<< ", data=" << current.signature << ".";
			SetSondeLastError(message.str());
			if (debug == true) {
				Test << "get_data_file_info signature mismatch at frame " << frame
					 << ": metrology=" << global_signature
					 << " data=" << current.signature << endl;
			}
			return err::kFrameSignatureMismatch;
		}

		for (int freq = 0; freq < config::kFreqCount; ++freq) {
			for (uint32_t tx = 0; tx < global_active_tx; ++tx) {
				if (!std::isfinite(current.rho_smt[freq][tx]) ||
					!std::isfinite(current.phase_smt[freq][tx]) ||
					!std::isfinite(current.DELTA_PH[freq][tx])) {
					std::ostringstream message;
					message << "Data frame " << frame << " contains a non-finite value at F"
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

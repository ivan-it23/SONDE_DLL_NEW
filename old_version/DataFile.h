#pragma once
// DataFile.h
// Определение формата файлов данных и проверка их принадлежности метрологии.

#include "Types.h"

// Анализирует файл данных после успешного sonde_set:
//   *.DEV — запись состоит из 11 служебных байт и GP_DATA;
//   *.bin — запись состоит только из GP_DATA.
// При успехе возвращает количество кадров, размер служебной части записи и
// сигнатуру данных. Каждый кадр проверяется на соответствие метрологии.
extern "C" __declspec(dllexport) int get_data_file_info(
	const char* dataPath,
	uint32_t* frameCount,
	int* frameHeaderSize,
	uint32_t* dataSignature);

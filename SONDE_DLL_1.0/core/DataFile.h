#pragma once
// DataFile.h
// Разбор файла данных прибора и проверка его соответствия загруженной метрологии.

#include "Types.h"

// Анализирует файл данных после успешного sonde_set:
//   *.DEV — запись состоит из 11 служебных байт и GP_DATA;
//   *.bin — запись состоит только из GP_DATA.
// Возвращает количество кадров, размер служебной части записи и сигнатуру данных.
// Каждый кадр проверяется на соответствие метрологии.
int scan_data_file(
	const char* dataPath,
	uint32_t* frameCount,
	int* frameHeaderSize,
	uint32_t* dataSignature);

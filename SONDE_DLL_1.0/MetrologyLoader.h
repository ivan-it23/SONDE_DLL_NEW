#pragma once
// MetrologyLoader.h
// Чтение бинарного файла метрологии и заполнение параметров зондов.
// Парсинг отделён от политики (проверка типа прибора, коды возврата),
// что позволяет переиспользовать его в sonde_set и ro_corr_ref_point.

#include "Types.h"

// Открывает и читает файл метрологии .bin.
// При успехе заполняет outMetro и outSignature, возвращает err::kOk.
// При неверном расширении или невозможности открыть файл логирует причину и
// возвращает err::kMetrologyFile.
int read_metrology_file(const char* path, GP_METROLOGY* outMetro, uint32_t* outSignature);

// Заполняет геометрические/частотные параметры зондов и поправки "нули воздуха"
// по содержимому метрологии. Применяет диаметр прибора по умолчанию, если он
// равен нулю.
void fill_sonde_params(const GP_METROLOGY& metrology, SONDE_PARAM param[2][5], float Air[2][5]);

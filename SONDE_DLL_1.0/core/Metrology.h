#pragma once
// Metrology.h
// Чтение и проверка бинарного файла метрологии, заполнение параметров зондов.
// Парсинг отделён от политики (проверка типа прибора, коды возврата), поэтому
// используется и в sonde_set, и в rho_corr_ref_point.

#include "Types.h"
#include "SondeIdentity.h"

// Открывает и читает файл метрологии .bin. При успехе заполняет outMetro и
// outSignature и возвращает err::kOk.
int read_metrology_file(const char* path, GP_METROLOGY* outMetro, uint32_t* outSignature);

// Проверяет 240-байтовую раскладку, тип прибора и геометрию активных зондов.
// При ошибке заполняет подробность sonde_get_last_error().
int validate_metrology(const GP_METROLOGY& metrology);

// Заполняет геометрию и рабочие частоты зондов, а также фазовые и амплитудные
// нули воздуха по содержимому метрологии.
void fill_sonde_params(const GP_METROLOGY& metrology, SONDE_PARAM param[2][5], float Air[2][5], float Air_att_dB[2][5]);

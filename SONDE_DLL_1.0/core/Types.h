#pragma once
// Types.h
// Бинарные структуры приборов и метрологии, а также рабочие структуры расчёта.
// Раскладка структур неизменна (#pragma pack(push,1)): они читаются и пишутся
// побайтово в бинарные файлы.

#include <complex>
#include <stdint.h>
#include <cstddef>

// Целочисленные типы заданы макросами: от них зависит раскладка бинарных структур.
#define int16_t short
#define uint32_t unsigned long
#define uint16_t unsigned short

using namespace std;

enum FREQ {
	_400_kGz,
	_2000_kGz,
};

enum T_SMT {
	T1, T2, T3, T4, T5
};

#pragma pack(push, 1)

// Геометрия и рабочая частота одного зонда.
struct SONDE_PARAM {
	float L1;
	float L2;
	float f;
};

// Калиброванный сигнал зонда: фазовый и амплитудный (затухание в дБ) каналы.
struct CAL_SIGNAL {
	float phase[2][5];   // симметризованные фазы [400, 2000][T1-T5], рад
	float att_dB[2][5];  // симметризованные затухания [400, 2000][T1-T5], дБ
	float Depth;
};

// УЭС по фазовому и амплитудному каналам + параметры зоны проникновения.
struct RHO {
	float rho_ph[2][5];  // фазовое УЭС [400, 2000][T1-T5]
	float rho_att[2][5]; // амплитудное УЭС [400, 2000][T1-T5]
	float rho_p[2];      // УЭС пласта
	float R_zp[2];       // радиус зоны проникновения, см
	float rho_zp[2];     // УЭС зоны проникновения
	float Depth;
};

// Разброс УЭС по группе зондов, %.
struct SERVICE {
	float delta_percent_min[2];
	float delta_percent_start[2];
};

struct GP_DATA {
	uint32_t signature;
	uint32_t condition;
	uint32_t frame;
	float temperature;
	float rho_smt[2][5];      // фазовые УЭС, рассчитанные на контроллере [400, 2000][T1-T5]
	float phase_smt[2][5];    // симметризованные фазы [400, 2000][T1-T5]
	float AM_RX_1[2][5];      // амплитуды на первом приемнике [400, 2000][T1-T5]
	float ZERO_AM_RX_1[2];    // амплитуды на первом приемнике [400, 2000] при молчащих передатчиках
	float AM_RX_2[2][5];      // амплитуды на втором приемнике [400, 2000][T1-T5]
	float ZERO_AM_RX_2[2];    // амплитуды на втором приемнике [400, 2000] при молчащих передатчиках
	float DELTA_PH[2][5];     // сырая разница фаз [400, 2000][T1-T5]
	float ZERO_dPH[2];        // разница фаз молчащих передатчиков
	float rho_att_smt[2][5];  // амплитудные УЭС, рассчитанные на контроллере [400, 2000][T1-T5]
	float att_smt_dB[2][5];   // симметризованные децибельные затухания [400, 2000][T1-T5]
};

struct GP_METROLOGY {
	uint32_t signature;
	uint32_t serial;
	uint16_t L1[5];
	uint16_t L2[5];
	uint16_t F[2];
	int16_t Air_ph[2][5];     // фазовые нули воздуха, милиградусы
	int16_t min_amp[2][5];    // минимальные амплитуды
	uint32_t D_sonde_mm;
	uint32_t work_type;
	uint32_t Rx_Position;     // 0-DEFAULT R1->T1, 1-R1->T2
	float Air_att_dB[2][5];   // амплитудные нули воздуха, дБ
	uint16_t service[58];     // на будущее до 240 байт
};

// Идентификатор прибора, декодированный из сигнатуры.
struct ID {
	uint32_t struct_size; // размер структуры данных прибора
	uint32_t type_;
	uint32_t N_Tx;
	uint32_t mod;
	uint32_t number;
	uint32_t type;
};

#pragma pack(pop)

static_assert(sizeof(GP_DATA) == 320, "GP_DATA binary layout must be 320 bytes");
static_assert(offsetof(GP_DATA, signature) == 0, "GP_DATA.signature offset mismatch");
static_assert(offsetof(GP_DATA, DELTA_PH) == 192, "GP_DATA.DELTA_PH offset mismatch");
static_assert(offsetof(GP_DATA, ZERO_dPH) == 232, "GP_DATA.ZERO_dPH offset mismatch");
static_assert(offsetof(GP_DATA, rho_att_smt) == 240, "GP_DATA.rho_att_smt offset mismatch");
static_assert(offsetof(GP_DATA, att_smt_dB) == 280, "GP_DATA.att_smt_dB offset mismatch");

static_assert(sizeof(GP_METROLOGY) == 240, "GP_METROLOGY binary layout must be 240 bytes");
static_assert(offsetof(GP_METROLOGY, signature) == 0, "GP_METROLOGY.signature offset mismatch");
static_assert(offsetof(GP_METROLOGY, L1) == 8, "GP_METROLOGY.L1 offset mismatch");
static_assert(offsetof(GP_METROLOGY, L2) == 18, "GP_METROLOGY.L2 offset mismatch");
static_assert(offsetof(GP_METROLOGY, F) == 28, "GP_METROLOGY.F offset mismatch");
static_assert(offsetof(GP_METROLOGY, Air_ph) == 32, "GP_METROLOGY.Air_ph offset mismatch");
static_assert(offsetof(GP_METROLOGY, min_amp) == 52, "GP_METROLOGY.min_amp offset mismatch");
static_assert(offsetof(GP_METROLOGY, D_sonde_mm) == 72, "GP_METROLOGY.D_sonde_mm offset mismatch");
static_assert(offsetof(GP_METROLOGY, work_type) == 76, "GP_METROLOGY.work_type offset mismatch");
static_assert(offsetof(GP_METROLOGY, Rx_Position) == 80, "GP_METROLOGY.Rx_Position offset mismatch");
static_assert(offsetof(GP_METROLOGY, Air_att_dB) == 84, "GP_METROLOGY.Air_att_dB offset mismatch");
static_assert(offsetof(GP_METROLOGY, service) == 124, "GP_METROLOGY.service offset mismatch");

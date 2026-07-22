#pragma once
// Types.h
// Доменные типы данных проекта SONDE_DLL: бинарные структуры приборов и
// метрологии и рабочие структуры расчёта.
// Раскладка структур должна оставаться неизменной (#pragma pack(push,1)),
// так как структуры читаются/пишутся побайтово в бинарные файлы.

#include <complex>
#include <vector>
#include <stdint.h>
#include <cstddef>

// Исходные макро-определения целочисленных типов сохранены без изменений:
// от них зависит раскладка бинарных структур приборов.
#define int16_t short
#define int32_t long
#define uint32_t unsigned long
#define Complex complex<long int>
#define uint16_t unsigned short
#define float16_t unsigned short

using namespace std;

// --------------------------------------------------------------------------
// Перечисления индексации частот и зондов.
// --------------------------------------------------------------------------
enum FREQ {
	_400_kGz,
	_2000_kGz,
};

enum T_CAL {
	T1_400, T2_400, T3_400, T4_400,
	T1_2000, T2_2000, T3_2000, T4_2000,
	T5_400, T5_2000,
};

enum T_SMT {
	T1, T2, T3, T4, T5
};

#pragma pack(push, 1)
struct Q_B {
	float QL1;
	float QL2;
};

struct SONDE_PARAM {
	float L1;
	float L2;
	float f;
	float M;
	float log_M;
	float D_sonde_m;
};

struct D_Border {
	double D_abs;
	double D_arg;
};

struct IQA {
	double I;
	double Q;
	double Angle;
};

struct PHASE {
	float Phase[2][5];
	float Depth;
};

struct Ro {
	float Ro[2][5];
	float Ro_p[2];
	float R_zp[2];
	float Ro_zp[2];
	float Depth;
};

struct ZP {
	float R_zp;
	float Ro_zp;
	float Ro_p;
	float tf;
};

//adjacent stratum AS
struct AS {
	float Ro_sonde;
	float Ro_up;
	float D;
	float tf;
};

struct SERVICE {
	float delta_percent_min[2];
	float delta_percent_start[2];
};

struct GP_DATA {
	uint32_t signature;
	uint32_t condition;
	uint32_t frame;
	float temperature;
	float rho_smt[2][5];      // УЭС, рассчитанные на контроллере [400, 2000][T1-T5]
	float phase_smt[2][5];    // симметризованные фазы [400, 2000][T1-T5]
	float AM_RX_1[2][5];      // амплитуды на первом приемнике [400, 2000][T1-T5]
	float ZERO_AM_RX_1[2];    // амплитуды на первом приемнике [400, 2000] при молчащих передатчиках
	float AM_RX_2[2][5];      // амплитуды на втором приемнике [400, 2000][T1 - T5]
	float ZERO_AM_RX_2[2];    // амплитуды на втором приемнике [400, 2000] при молчащих передатчиках
	float DELTA_PH[2][5];     // сырая разница фаз [400, 2000][T1 - T5]
	float ZERO_dPH[2];        // разница фаз молчащих передатчиков
};

struct GP_METROLOGY {
	uint32_t signature;
	uint32_t serial;
	uint16_t L1[5];
	uint16_t L2[5];
	uint16_t F[2];
	int16_t Air_zz[2][5];
	int16_t Air_zz_amt[2][5];
	uint32_t D_sonde_mm;
	uint32_t work_type;
	uint32_t Rx_Position; // 0: R1->T1, 1: R1->T2
	uint16_t service[78]; // резерв до 240 байт
};

struct INF_CYL {
	complex <float> sonde[2][5];
};

struct ID {
	uint32_t type_;
	uint32_t N_Tx;
	uint32_t mod;
	uint32_t number;
	uint32_t type;
};

#pragma pack(pop)

static_assert(sizeof(GP_DATA) == 240, "GP_DATA binary layout must be 240 bytes");
static_assert(offsetof(GP_DATA, signature) == 0, "GP_DATA.signature offset mismatch");
static_assert(offsetof(GP_DATA, DELTA_PH) == 192, "GP_DATA.DELTA_PH offset mismatch");
static_assert(offsetof(GP_DATA, ZERO_dPH) == 232, "GP_DATA.ZERO_dPH offset mismatch");

static_assert(sizeof(GP_METROLOGY) == 240, "GP_METROLOGY binary layout must be 240 bytes");
static_assert(offsetof(GP_METROLOGY, signature) == 0, "GP_METROLOGY.signature offset mismatch");
static_assert(offsetof(GP_METROLOGY, L1) == 8, "GP_METROLOGY.L1 offset mismatch");
static_assert(offsetof(GP_METROLOGY, L2) == 18, "GP_METROLOGY.L2 offset mismatch");
static_assert(offsetof(GP_METROLOGY, F) == 28, "GP_METROLOGY.F offset mismatch");
static_assert(offsetof(GP_METROLOGY, Air_zz) == 32, "GP_METROLOGY.Air_zz offset mismatch");
static_assert(offsetof(GP_METROLOGY, Air_zz_amt) == 52, "GP_METROLOGY.Air_zz_amt offset mismatch");
static_assert(offsetof(GP_METROLOGY, D_sonde_mm) == 72, "GP_METROLOGY.D_sonde_mm offset mismatch");
static_assert(offsetof(GP_METROLOGY, work_type) == 76, "GP_METROLOGY.work_type offset mismatch");
static_assert(offsetof(GP_METROLOGY, Rx_Position) == 80, "GP_METROLOGY.Rx_Position offset mismatch");
static_assert(offsetof(GP_METROLOGY, service) == 84, "GP_METROLOGY.service offset mismatch");

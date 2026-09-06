#pragma once

// Публичный ABI-контракт SONDE_DLL_NEW для подключения к приложению заказчика.
// Бинарные файлы и структуры используют little-endian раскладку Windows.

#include <stdint.h>

#if defined(SONDEDLL10_EXPORTS)
#define SONDE_API __declspec(dllexport)
#else
#define SONDE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
#define SONDE_EXTERN_C extern "C"
using SONDE_BOOL = bool;
#else
#include <stdbool.h>
#define SONDE_EXTERN_C
typedef bool SONDE_BOOL;
#endif

#pragma pack(push, 1)

typedef struct SONDE_PHASE {
	float Phase[2][5];
	float Depth;
} SONDE_PHASE;

typedef struct SONDE_RO {
	float Ro[2][5];
	float Ro_p[2];
	float R_zp[2];
	float Ro_zp[2];
	float Depth;
} SONDE_RO;

typedef struct SONDE_SERVICE {
	float delta_percent_min[2];
	float delta_percent_start[2];
} SONDE_SERVICE;

// Исторические имена структур сохранены, чтобы существующий код заказчика
// продолжал компилироваться без переименований.
typedef SONDE_PHASE PHASE;
typedef SONDE_RO Ro;
typedef SONDE_SERVICE SERVICE;

typedef struct SONDE_GP_DATA {
	uint32_t signature;
	uint32_t condition;
	uint32_t frame;
	float temperature;
	float rho_smt[2][5];
	float phase_smt[2][5];
	float AM_RX_1[2][5];
	float ZERO_AM_RX_1[2];
	float AM_RX_2[2][5];
	float ZERO_AM_RX_2[2];
	float DELTA_PH[2][5];
	float ZERO_dPH[2];
} SONDE_GP_DATA;
typedef SONDE_GP_DATA GP_DATA;

typedef struct SONDE_METROLOGY {
	uint32_t signature;
	uint32_t serial;
	uint16_t L1[5];
	uint16_t L2[5];
	uint16_t F[2];
	int16_t Air_zz[2][5];
	int16_t Air_zz_amt[2][5];
	uint32_t D_sonde_mm;
	uint32_t work_type;
	uint32_t Rx_Position;
	uint16_t service[78];
} SONDE_METROLOGY;
typedef SONDE_METROLOGY GP_METROLOGY;

#pragma pack(pop)

enum SONDE_ERROR {
	SONDE_OK = 0,
	SONDE_METROLOGY_FILE = 1,
	SONDE_FRAME_SIGNATURE_MISMATCH = 2,
	SONDE_DATA_FILE = 3,
	SONDE_DATA_FILE_EXTENSION = 4,
	SONDE_DATA_FILE_LAYOUT = 5,
	SONDE_INVALID_ARGUMENT = 6,
	SONDE_NOT_INITIALIZED = 7,
	SONDE_METROLOGY_SIZE = 8,
	SONDE_METROLOGY_LAYOUT = 9,
	SONDE_METROLOGY_GEOMETRY = 10,
	SONDE_METROLOGY_RX_POSITION = 11,
	SONDE_NUMERICAL_FAILURE = 12,
	SONDE_UNSUPPORTED_TYPE = 100,
	SONDE_NEURO_DLL_NOT_LOADED = 200,
	SONDE_NEURO_FUNCTION_NOT_FOUND = 201,
	SONDE_NEURO_CREATE_FAILED = 202,
	SONDE_NEURO_WEIGHTS_NOT_FOUND = 203,
	SONDE_NEURO_PREDICT_FAILED = 300,
	SONDE_NEURO_NOT_INITIALIZED = 301
};

// Поддерживаются: автономный 4/5Tx, LWD 3/4Tx, картограф LWD 4Tx и временно 351.
// Для нейросетевых LWD/картограф 4Tx требуется каталог весов с суффиксом -XYZ,
// где XYZ — три цифры типа прибора из сигнатуры метрологии.
SONDE_EXTERN_C SONDE_API int sonde_set(void* metrology_path, const char* reserved);
SONDE_EXTERN_C SONDE_API int get_data_file_info(const char* data_path, uint32_t* frame_count, int* frame_header_size, uint32_t* data_signature);
SONDE_EXTERN_C SONDE_API int get_express_data(void* data, PHASE* phase, Ro* rho, int shift);
SONDE_EXTERN_C SONDE_API int get_Phase(void* data, PHASE* phase, int shift);
SONDE_EXTERN_C SONDE_API int get_condition(void* data, uint32_t* condition, int shift);
SONDE_EXTERN_C SONDE_API int simmetry(PHASE* phase_in, PHASE* phase_smt, uint32_t condition);
SONDE_EXTERN_C SONDE_API int calculate_Rho_AF(PHASE* phase, Ro* rho, float ro_bh, int borehole_diameter_mm, int pz_400, int pz_2000, SERVICE* service);
SONDE_EXTERN_C SONDE_API int calculate_Rho_Doll_GR(PHASE* phase, Ro* rho);
SONDE_EXTERN_C SONDE_API int borehole_offset(float ro_bh, int borehole_diameter_mm);
SONDE_EXTERN_C SONDE_API int ph_shift_smt_ph(PHASE* phase, Ro* required_rho, PHASE* phase_shift);
SONDE_EXTERN_C SONDE_API int ph_shift_smt_ro(Ro* calculated_rho, Ro* required_rho, PHASE* phase_shift);
SONDE_EXTERN_C SONDE_API int ro_corr_ref_point(void* metrology_path, Ro* calculated_reference, Ro* required_reference, Ro* calculated, Ro* required);
SONDE_EXTERN_C SONDE_API int ph_smt_ro(Ro* calculated_rho, PHASE* phase);
SONDE_EXTERN_C SONDE_API int ph_smt_zp(Ro* rho, PHASE* phase);
SONDE_EXTERN_C SONDE_API int anti_spiral(double* input, double* output, int length, int fourier_window, int moving_average_window);
SONDE_EXTERN_C SONDE_API void debug_mode(SONDE_BOOL enabled);

// Указатель действует до следующего вызова DLL в том же потоке; строку не освобождать.
SONDE_EXTERN_C SONDE_API const char* sonde_get_last_error(void);

#ifdef __cplusplus
static_assert(sizeof(SONDE_GP_DATA) == 240, "SONDE_GP_DATA must be 240 bytes");
static_assert(sizeof(SONDE_METROLOGY) == 240, "SONDE_METROLOGY must be 240 bytes");
static_assert(sizeof(SONDE_PHASE) == 44, "SONDE_PHASE ABI mismatch");
static_assert(sizeof(SONDE_RO) == 68, "SONDE_RO ABI mismatch");
#endif

#undef SONDE_EXTERN_C

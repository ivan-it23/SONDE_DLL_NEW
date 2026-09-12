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

// Калиброванный сигнал зонда: фазовый (phase, рад) и амплитудный (att_dB, дБ) каналы.
typedef struct SONDE_CAL_SIGNAL {
	float phase[2][5];
	float att_dB[2][5];
	float Depth;
} SONDE_CAL_SIGNAL;

// УЭС по фазовому и амплитудному каналам + параметры зоны проникновения.
typedef struct SONDE_RHO {
	float rho_ph[2][5];
	float rho_att[2][5];
	float rho_p[2];
	float R_zp[2];
	float rho_zp[2];
	float Depth;
} SONDE_RHO;

// Разброс УЭС по группе зондов, %.
typedef struct SONDE_SERVICE {
	float delta_percent_min[2];
	float delta_percent_start[2];
} SONDE_SERVICE;

// Публичные имена структур, ожидаемые кодом заказчика.
typedef SONDE_CAL_SIGNAL CAL_SIGNAL;
typedef SONDE_RHO RHO;
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
	float rho_att_smt[2][5];
	float att_smt_dB[2][5];
} SONDE_GP_DATA;
typedef SONDE_GP_DATA GP_DATA;

typedef struct SONDE_METROLOGY {
	uint32_t signature;
	uint32_t serial;
	uint16_t L1[5];
	uint16_t L2[5];
	uint16_t F[2];
	int16_t Air_ph[2][5];
	int16_t min_amp[2][5];
	uint32_t D_sonde_mm;
	uint32_t work_type;
	uint32_t Rx_Position;
	float Air_att_dB[2][5];
	uint16_t service[58];
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

// Поддерживаются: автономный 4/5Tx, LWD 3/4Tx, картограф LWD 4Tx и 351.
// Для нейросетевых LWD/картограф 4Tx требуется каталог весов с суффиксом -XYZ,
// где XYZ — три цифры типа прибора из сигнатуры метрологии.

// Загружает файл метрологии и для нейросетевых типов инициализирует предиктор.
SONDE_EXTERN_C SONDE_API int sonde_set(void* metrology_path);

// Возвращает число кадров, размер служебной части записи и сигнатуру файла данных.
SONDE_EXTERN_C SONDE_API int get_data_file_info(const char* data_path, uint32_t* frame_count, int* frame_header_size, uint32_t* data_signature);

// Возвращает готовые с контроллера симметризованные сигналы и УЭС обоих каналов.
SONDE_EXTERN_C SONDE_API int get_express_data(void* data, CAL_SIGNAL* cal_signal, RHO* rho, int shift);

// Возвращает калиброванные на воздух фазы и амплитудные затухания в дБ.
SONDE_EXTERN_C SONDE_API int get_cal_signal(void* data, CAL_SIGNAL* cal_signal, int shift);

// Возвращает байт работоспособности передатчиков.
SONDE_EXTERN_C SONDE_API int get_condition(void* data, uint32_t* condition, int shift);

// Симметризует фазовый и амплитудный каналы по байту работоспособности.
SONDE_EXTERN_C SONDE_API int simmetry(CAL_SIGNAL* cal_signal_in, CAL_SIGNAL* cal_signal_smt, uint32_t condition);

// УЭС по фазе и по затуханию, без учёта скважины и зоны проникновения.
SONDE_EXTERN_C SONDE_API int calculate_rho(CAL_SIGNAL* cal_signal, RHO* rho);

// Истинное УЭС пласта и параметры зоны проникновения нейросетью.
// Принимает симметризованный сигнал; заполняет rho_ph, rho_p, rho_zp, R_zp.
SONDE_EXTERN_C SONDE_API int calculate_true_rho_neuro(CAL_SIGNAL* cal_signal, RHO* rho, SERVICE* service);

// Корректирует УЭС искомой точки по опорной точке. Работает без sonde_set.
SONDE_EXTERN_C SONDE_API int rho_corr_ref_point(void* metrology_path, RHO* calculated_reference, RHO* required_reference, RHO* calculated, RHO* required);

// Восстанавливает из УЭС симметризованные фазу и затухание в дБ.
SONDE_EXTERN_C SONDE_API int signal_smt_from_ro(RHO* calculated_rho, CAL_SIGNAL* cal_signal);

// Модельные симметризованные фазы для параметров зоны проникновения.
SONDE_EXTERN_C SONDE_API int ph_smt_zp(RHO* rho, CAL_SIGNAL* cal_signal);

// Подавление спиральной помехи.
SONDE_EXTERN_C SONDE_API int anti_spiral(double* input, double* output, int length, int fourier_window, int moving_average_window);

// Включает и выключает запись отладочного лога Test.txt.
SONDE_EXTERN_C SONDE_API void debug_mode(SONDE_BOOL enabled);

// Указатель действует до следующего вызова DLL в том же потоке; строку не освобождать.
SONDE_EXTERN_C SONDE_API const char* sonde_get_last_error(void);

#ifdef __cplusplus
static_assert(sizeof(SONDE_GP_DATA) == 320, "SONDE_GP_DATA must be 320 bytes");
static_assert(sizeof(SONDE_METROLOGY) == 240, "SONDE_METROLOGY must be 240 bytes");
static_assert(sizeof(SONDE_CAL_SIGNAL) == 84, "SONDE_CAL_SIGNAL ABI mismatch");
static_assert(sizeof(SONDE_RHO) == 108, "SONDE_RHO ABI mismatch");
#endif

#undef SONDE_EXTERN_C

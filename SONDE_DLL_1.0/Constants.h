#pragma once
// Constants.h
// Централизованные конфигурационные и физические константы проекта SONDE_DLL.


#include <cmath>

// --------------------------------------------------------------------------
// Физические константы и коэффициенты пересчёта единиц.
// --------------------------------------------------------------------------
const double PI = 3.1415927410125732;
const double eps0 = 8.85 * 1e-12;
const double mu0 = 4 * PI * 1e-7;
const float mV = 2500.0f / 268435456.0f;
const float mG = static_cast<float>(1000.0 * 180.0 / PI);
const float sG = static_cast<float>(200.0 * 180.0 / PI); // цена дискреты = 0.005 градуса
const float Grad = static_cast<float>(180.0 / PI);

// --------------------------------------------------------------------------
// Сигнатурные коды типов приборов ЭМК.
// --------------------------------------------------------------------------
#define CARTOGRAPH         351
#define CARTOGRAPH_LWD_4Tx 359
#define AUTONOM_4Tx        141
#define AUTONOM_5Tx        151
#define AUTONOM_5Tx_SDR    152
#define LWD_3Tx            231
#define LWD_4Tx            241
#define LWD_4Tx_NEW        242

// --------------------------------------------------------------------------
// Прикладные конфигурационные константы.
// --------------------------------------------------------------------------
namespace config {

// Размерности измерительной схемы прибора.
constexpr int kFreqCount = 2; // число рабочих частот (400 / 2000 кГц)
constexpr int kMaxTx = 5;     // максимальное число передатчиков (T1..T5)

// Нейросетевой предиктор (NEURO_TEST.dll).
constexpr int kNeuroInputCount = 8;  // 4 Tx * 2 частоты
constexpr int kNeuroOutputCount = 3; // Ro_p, Ro_zp, R_zp
constexpr char kNeuroDllName[] = "NEURO_TEST.dll";
constexpr char kNeuroWeightsDir[] = "exported_weights_PINN_symm";
constexpr char kNeuroCreateFn[] = "GeoPredictor_Create";
constexpr char kNeuroPredictFn[] = "GeoPredictor_Predict";
constexpr char kNeuroDestroyFn[] = "GeoPredictor_Destroy";
constexpr char kNeuroLastErrorFn[] = "GeoPredictor_GetLastError";

// Логирование.
constexpr char kLogFileName[] = "Test.txt";

// Метрология.
constexpr int kDefaultSondeDiameterMm = 90; // диаметр прибора по умолчанию (autonomy 1DDS)
constexpr char kMetrologyExtension[] = "bin";

// Решатель УЭС методом золотого сечения (RO_dFI).
constexpr float kRoSolverMin = 0.01f;          // минимальное УЭС поиска
constexpr float kRoSolverMax = 7000.0f;        // максимальное УЭС поиска
constexpr float kGoldenEpsilon = 0.0000005f;   // точность по фазе/амплитуде
constexpr float kGoldenFactor = 0.382f;        // коэффициент золотого сечения

// Компенсация влияния скважины (DFI_bhole).
constexpr float kSondeRadiusM = 0.06f; // эффективный радиус прибора

// Маркер недопустимой фазы (ph_smt_ro).
constexpr float kInvalidPhase = -32768.00f;

} // namespace config

// --------------------------------------------------------------------------
// Коды возврата экспортируемых функций.
// --------------------------------------------------------------------------
namespace err {

constexpr int kOk = 0;
constexpr int kMetrologyFile = 1;        // файл метрологии не открыт или не .bin
constexpr int kFrameSignatureMismatch = 2; // сигнатура кадра != сигнатуре метрологии
constexpr int kUnsupportedType = 100;    // неподдерживаемый тип прибора
constexpr int kNeuroDllNotLoaded = 200;  // NEURO_TEST.dll не загружена
constexpr int kNeuroFuncNotFound = 201;  // не найдены функции нейросети
constexpr int kNeuroCreateFailed = 202;  // предиктор не создан
constexpr int kNeuroPredictFailed = 300; // ошибка предсказания
constexpr int kNeuroNotInitialized = 301; // предиктор не инициализирован

} // namespace err

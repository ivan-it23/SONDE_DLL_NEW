# SONDE_DLL — документация по двум проектам

Актуально на 2026-09-06. Документ покрывает **оба** проекта — расчётную библиотеку
`SONDE_DLL_NEW` (DLL) и отладочный потребитель `SONDE_DLL_TEST_NEW` (TEST) — их
архитектуру, форматы данных, экспортируемый API, особенности и связанные
эталонные проекты коллеги.

---

## 1. Назначение и общая картина

Система обрабатывает данные приборов **электромагнитного каротажа (ЭМК)** —
многозондовых индукционных приборов (автономные, LWD, картограф). Прибор пишет
бинарные кадры (фазы, амплитуды по 2 частотам × до 5 передатчиков); DLL
преобразует их в удельное электрическое сопротивление (УЭС) и параметры зоны
проникновения.

Два проекта:

| Проект | Роль | Тип | Репозиторий |
|---|---|---|---|
| `SONDE_DLL_NEW` | Расчётная DLL (`SONDE_DLL_1.0.dll`) | Windows DLL, C++ | `github.com/ivan-it23/SONDE_DLL_NEW` |
| `SONDE_DLL_TEST_NEW` | Потребитель/отладчик DLL | C++/CLI WinForms (.NET 4.7.2) | `github.com/ivan-it23/SONDE_DLL_TEST_NEW` |

TEST загружает DLL через `LoadLibrary`/`GetProcAddress`, прогоняет файлы данных и
рисует результаты на графиках (Windows.Forms.DataVisualization.Charting).

### Отличие нашей реализации от эталона

Расчёт зоны проникновения у нас выполняет **нейросеть** (`NEURO_TEST.dll`), а не
палетки. Это осознанное архитектурное решение: от палеток уходим. Везде, где в
эталоне коллеги палеточная логика, у нас — нейросетевой путь. Это **не ошибка**.

---

## 2. Эталонные проекты коллеги (для спорных моментов)

Коллега развивает исходные версии; при расхождениях сверяемся с ними. Скачаны с
GitHub как zip ветки `master`, лежат на рабочем столе (не git-репозитории
локально):

| Эталон | Путь | Соответствует |
|---|---|---|
| DLL коллеги | `C:\Users\Admin\Desktop\SONDE_DLL_PH_ATT_RHO_NEURO-master` | `SONDE_DLL_NEW` |
| TEST коллеги | `C:\Users\Admin\Desktop\SONDE_DLL_PH_ATT_RHO_NEURO_TEST-master` | `SONDE_DLL_TEST_NEW` |

Имена репозиториев коллеги: `SONDE_DLL_PH_ATT_RHO_NEURO` и
`SONDE_DLL_PH_ATT_RHO_NEURO_TEST` (точный владелец/URL уточнять у коллеги).

**Правило сверки:** логика и функционал должны точь-в-точь совпадать с коллегой,
**за исключением** палеток → нейросеть.

### ⚠️ Предупреждение по коду коллеги

В `SONDE_DLL_PH_ATT_RHO_NEURO-master/SONDE_DLL_1.0/SONDE_DLL_1.0.cpp` присутствует
закомментированный сетевой «лицензионный» блок (XOR-обфускация строк, скрытая
загрузка `wininet.dll`, FTP-выгрузка серийника диска C на `ftp.site.com`,
анти-отладка `IsDebuggerPresent`→`TerminateProcess`, TLS-callback
`SilentCheckThread`, заглушка-экспорт `DoWork`). Он **не имеет отношения к
каротажу** и **никогда не переносится** в наши проекты ни в каком виде.

---

## 3. Форматы данных (бинарная раскладка — критично)

Все структуры обмена — `#pragma pack(push,1)`, little-endian. DLL и TEST хранят
**идентичную** раскладку; она закреплена `static_assert`-ами в обоих проектах.
Публичный ABI DLL — в [SondeApi.h](SONDE_DLL_1.0/api/SondeApi.h), внутренние типы — в
[Types.h](SONDE_DLL_1.0/core/Types.h), у TEST — в
`SONDE_DLL_TEST_NEW/SONDE_1.0/variable.h`.

### 3.1. Канальная модель: фаза + амплитуда

Ключевая особенность текущей версии — **два канала измерений**:
- **фазовый** — разность фаз между приёмниками (рад);
- **амплитудный** — затухание сигнала `20·log10(AM_RX_2/AM_RX_1)` (дБ).

### 3.2. `CAL_SIGNAL` (84 байта) — калиброванный сигнал зонда

```c
struct CAL_SIGNAL {
    float phase[2][5];   // фазы   [частота 400/2000][T1..T5], рад
    float att_dB[2][5];  // затухания [частота][T], дБ
    float Depth;
};
```

### 3.3. `RHO` (108 байт) — УЭС и параметры зоны проникновения

```c
struct RHO {
    float rho_ph[2][5];  // УЭС по фазе        [частота][T]
    float rho_att[2][5]; // УЭС по затуханию   [частота][T]
    float rho_p[2];      // УЭС пласта (зона проникновения)
    float R_zp[2];       // радиус зоны проникновения, СМ
    float rho_zp[2];     // УЭС зоны проникновения
    float Depth;
};
```

`rho_p`/`rho_zp`/`R_zp` заполняются одинаково для обеих частот (это свойства
среды, различие сигналов даёт геометрия/частота зонда). `R_zp` хранится в **см**;
`ph_smt_zp` переводит обратно в метры.

### 3.4. `GP_DATA` (320 байт) — кадр прибора

```c
struct GP_DATA {
    uint32_t signature, condition, frame; float temperature;   // 0..15
    float rho_smt[2][5];      // 16   фазовые УЭС с контроллера
    float phase_smt[2][5];    // 56   симметризованные фазы
    float AM_RX_1[2][5];      // 96   амплитуды приёмник 1
    float ZERO_AM_RX_1[2];    // 136
    float AM_RX_2[2][5];      // 144  амплитуды приёмник 2
    float ZERO_AM_RX_2[2];    // 184
    float DELTA_PH[2][5];     // 192  сырая разница фаз
    float ZERO_dPH[2];        // 232
    float rho_att_smt[2][5];  // 240  амплитудные УЭС с контроллера   (НОВОЕ)
    float att_smt_dB[2][5];   // 280  симметризованные затухания, дБ  (НОВОЕ)
};
```

Старая прошивка шлёт кадр 240 байт (без двух последних массивов), новая — 320.
Совместимость обеспечивается полем `struct_size` (см. §4).

### 3.5. `GP_METROLOGY` (240 байт) — файл метрологии `.bin`

```c
struct GP_METROLOGY {
    uint32_t signature, serial;
    uint16_t L1[5], L2[5], F[2];   // геометрия зондов (мм) и частоты
    int16_t  Air_ph[2][5];         // фазовые нули воздуха     (было Air_zz)
    int16_t  min_amp[2][5];        // мин. амплитуды           (было Air_zz_amt)
    uint32_t D_sonde_mm, work_type, Rx_Position;
    float    Air_att_dB[2][5];     // амплитудные нули воздуха, дБ   (НОВОЕ)
    uint16_t service[58];          // резерв до 240 байт
};
```

`Rx_Position`: 0 — R1→T1 (по умолчанию), 1 — R1→T2.

### 3.6. `ID` — декодированный идентификатор прибора

```c
struct ID { uint32_t struct_size, type_, N_Tx, mod, number, type; };
```

---

## 4. Сигнатура прибора и `get_sonde_id`

Сигнатура — десятичное число. Младшие 6 разрядов кодируют прибор, старшие
(`signature / 1000000`) — **размер структуры кадра** (версионирование).

Разбор ([SondeIdentity.cpp](SONDE_DLL_1.0/core/SondeIdentity.cpp), в TEST —
`function.cpp`):

```
type   = (signature % 1000000) / 1000     // напр. 349, 241
type_  = (signature % 1000000) / 100000   // семейство: 1 автоном, 2 LWD, 3 картограф
N_Tx   = (signature % 100000)  / 10000    // число передатчиков
mod    = (signature % 10000)   / 1000
number =  signature % 1000
struct_size = signature / 1000000  (если 0 — известным типам присваивается 240)
```

Сравнение «кадр ↔ метрология» идёт **по `% 1000000`** (игнорируя разряды
`struct_size`), т.к. в метрологии старшие разряды могут быть нулевыми.

При чтении кадра копируется `min(struct_size, sizeof(GP_DATA))` байт в
обнулённую `GP_DATA` — безопасно и для 240-, и для 320-байтовых кадров. Так делают
и DLL (`read_frame` в [SignalCalibration.cpp](SONDE_DLL_1.0/core/SignalCalibration.cpp),
`scan_data_file` в [DataFile.cpp](SONDE_DLL_1.0/core/DataFile.cpp)), и TEST
(чтение кадра в `ProcessDataFiles`).

### Коды типов приборов ([Constants.h](SONDE_DLL_1.0/core/Constants.h))

| Макрос | Код | Нейросеть |
|---|---|---|
| `AUTONOM_4Tx` / `AUTONOM_5Tx` / `AUTONOM_5Tx_SDR` | 141 / 151 / 152 | нет |
| `LWD_3Tx` / `LWD_4Tx` / `LWD_4Tx_NEW` | 231 / 241 / 242 | 4Tx — да |
| `CARTOGRAPH_LWD_4Tx` | **349** | да |
| `CARTOGRAPH` | 351 | совместимость |

> **Важно:** `CARTOGRAPH_LWD_4Tx = 349** (не 359). Ранее в нашем проекте было
> ошибочно 359 — исправлено; каталоги весов переименованы в `*-349`. Подтверждено
> пользователем.

Нейросеть допускается при `N_Tx == 4 && (type_ == 2 || type_ == 3)`.

---

## 5. Проект DLL — `SONDE_DLL_NEW`

### 5.1. Модульная архитектура

Исходники разложены по трём каталогам с однозначным правилом:
**экспортируется → `api/`, считает физику → `core/`, обращается к Windows или
внешней DLL → `platform/`.**

```
SONDE_DLL_1.0/
├── api/         единственная точка экспорта
├── core/        расчётное ядро, экспортов нет
├── platform/    инфраструктура и внешние зависимости
└── stdafx.h/.cpp, targetver.h, *.vcxproj
```

| Файл | Ответственность |
|---|---|
| [api/SondeApi.h](SONDE_DLL_1.0/api/SondeApi.h) | Публичный ABI: структуры, коды ошибок, прототипы |
| [api/SondeApi.cpp](SONDE_DLL_1.0/api/SondeApi.cpp) | Все 14 экспортируемых функций; блокировка состояния и делегирование в `core` |
| [core/Types.h](SONDE_DLL_1.0/core/Types.h) / [core/Constants.h](SONDE_DLL_1.0/core/Constants.h) | Бинарные структуры; константы, коды типов и коды ошибок |
| [core/SondeIdentity.cpp](SONDE_DLL_1.0/core/SondeIdentity.cpp) | `get_sonde_id`, `ToolCapabilities` |
| [core/SondeState.cpp](SONDE_DLL_1.0/core/SondeState.cpp) | Глобальное состояние + `recursive_mutex` |
| [core/Metrology.cpp](SONDE_DLL_1.0/core/Metrology.cpp) | Чтение/валидация метрологии, `fill_sonde_params` |
| [core/SignalCalibration.cpp](SONDE_DLL_1.0/core/SignalCalibration.cpp) | Разбор кадра, калибровка фаз/затуханий, матрицы K, симметризация |
| [core/Resistivity.cpp](SONDE_DLL_1.0/core/Resistivity.cpp) | `SIGNAL`, `RO_ARG`, `RO_ATT`, расчёт и коррекция УЭС |
| [core/NeuroResistivity.cpp](SONDE_DLL_1.0/core/NeuroResistivity.cpp) | Истинное УЭС и параметры зоны проникновения нейросетью |
| [core/InvasionForward.cpp](SONDE_DLL_1.0/core/InvasionForward.cpp) | Прямая задача двухслойной цилиндрической модели (`Vzz_inf_cyl`, Бессель) |
| [core/DataFile.cpp](SONDE_DLL_1.0/core/DataFile.cpp) | `scan_data_file` |
| [core/AntiSpiral.cpp](SONDE_DLL_1.0/core/AntiSpiral.cpp) | Подавление спиральной помехи |
| [platform/NeuroPredictor.cpp](SONDE_DLL_1.0/platform/NeuroPredictor.cpp) | Загрузка `NEURO_TEST.dll`, поиск весов, предсказание |
| [platform/Logger.cpp](SONDE_DLL_1.0/platform/Logger.cpp) / [platform/ErrorState.cpp](SONDE_DLL_1.0/platform/ErrorState.cpp) | Потокобезопасный лог `Test.txt`; `thread_local` последняя ошибка |
| [platform/dllmain.cpp](SONDE_DLL_1.0/platform/dllmain.cpp) | Точка входа DLL |

Сборка: Visual Studio, toolset v145, конфигурации Debug/Release × x64/Win32(x86),
`ConfigurationType=DynamicLibrary`, `SONDEDLL10_EXPORTS`.

### 5.2. Экспортируемый API (14 функций)

Все — `extern "C"`, коды возврата `int` (0 = успех), кроме указанного.

| Функция | Сигнатура | Назначение |
|---|---|---|
| `sonde_set` | `(void* metrology_path)` | Загрузка метрологии + инициализация нейросети |
| `get_data_file_info` | `(const char* path, uint32_t* frames, int* header, uint32_t* sig)` | Проверка файла данных, число кадров, размер служебной части |
| `get_express_data` | `(void* data, CAL_SIGNAL*, RHO*, int shift)` | Готовые с контроллера сигналы и УЭС (оба канала) |
| `get_cal_signal` | `(void* data, CAL_SIGNAL*, int shift)` | Калибровка сырых данных: фазы (`DELTA_PH − Air_ph`) и затухания (`20·log10(AM2/AM1) − Air_att_dB`), знак чередуется по зондам |
| `get_condition` | `(void* data, uint32_t*, int shift)` | Байт работоспособности передатчиков |
| `simmetry` | `(CAL_SIGNAL* in, CAL_SIGNAL* smt, uint32_t condition)` | Симметризация обоих каналов матрицей K по работоспособности |
| `calculate_rho` | `(CAL_SIGNAL*, RHO*)` | УЭС по фазе (`RO_ARG`) и по затуханию (`RO_ATT`) для всех зондов |
| `calculate_true_rho_neuro` | `(CAL_SIGNAL*, RHO*, SERVICE*)` | Фазовые УЭС по зондам + **нейросетевые** `rho_p/rho_zp/R_zp`; в `SERVICE` — разброс УЭС по зондам, % |
| `rho_corr_ref_point` | `(void* metro, RHO* calk_ref, RHO* need_ref, RHO* calk, RHO* required)` | Посадка на опорную точку по обоим каналам; работает без `sonde_set` |
| `signal_smt_from_ro` | `(RHO*, CAL_SIGNAL*)` | Обратно из УЭС в симметризованный сигнал (операция «КАРАНДАШ»); вне диапазона — маркер `-32768` |
| `ph_smt_zp` | `(RHO*, CAL_SIGNAL*)` | Модельные фазы для параметров зоны проникновения (прямая задача, нейро-результат) |
| `anti_spiral` | `(double* in, double* out, int len, int win_f, int win_ma)` | Подавление спиральной помехи |
| `debug_mode` | `(bool)` → void | Вкл/выкл лог `Test.txt` |
| `sonde_get_last_error` | `()` → `const char*` | Текст последней ошибки (thread-local) |

### 5.3. Численные методы

- `SIGNAL(param, ro)` — комплексный сигнал зонда однородной среды;
  `arg` → фазовый сдвиг, `abs` → амплитудное отношение.
- `RO_ARG` — УЭС от фазы, `RO_ATT` — УЭС от затухания; оба методом золотого
  сечения (`kGoldenFactor=0.382`, `kGoldenEpsilon=5e-7`; диапазон фазы
  0.01..7000, амплитуды 0.01..1000). Итерации ограничены страховочным потолком.
- Симметризация — матрицы `K[5][5]` (`formula_simmetry`) по числу передатчиков и
  байту работоспособности; применяется к обоим каналам.
- Зона проникновения — нейросеть (см. §5.4); прямая проверка — `Vzz_inf_cyl`
  (интеграл с функциями Бесселя) в `ph_smt_zp`.

### 5.4. Интеграция нейросети

`NEURO_TEST.dll` рядом с `SONDE_DLL_1.0.dll`. Экспорты: `GeoPredictor_Create`
(каталог весов), `GeoPredictor_Predict` (8 входов → 3 выхода),
`GeoPredictor_Destroy`, `GeoPredictor_GetLastError`.

- **Входы (8):** симметризованные фазы 4 зондов × 2 частоты, в градусах
  (`phase * Grad`).
- **Выходы (3):** `out[0]=r_inv` (м), `out[1]=rho_inv` (Ом·м), `out[2]=rho_form`
  (Ом·м) → в `RHO`: `R_zp = r_inv*100` (см), `rho_zp = rho_inv`, `rho_p = rho_form`.

**Веса:** каталог `neuro-weights/<имя>-<XYZ>/`, где `XYZ` — код типа прибора
(3 цифры: `X` — тип/семейство, `Y` — число передатчиков, `Z` — модификация).
Каждый комплект — 12 файлов: `w1..w4`, `b1..b4`, `in_mean`, `in_scale`,
`out_min`, `out_scale`. Имеющиеся: `LWD_4Tx-241`, `LWD_4Tx_NEW-242`,
`CARTOGRAPH_LWD_4Tx-349`. Продублированы в `runtime/win-x64` и `runtime/win-Win32`
(выведены из-под `.gitignore` для поставки).

**Выбор каталога весов** (`BuildWeightsDir` в
[NeuroPredictor.cpp](SONDE_DLL_1.0/platform/NeuroPredictor.cpp)): **модификация (3-я цифра
`Z`) не влияет на выбор**. Обязателен матч по первым двум цифрам — тип прибора
`X` + число передатчиков `Y` (совпадают с сигнатурой из файла данных). Точное
совпадение всех трёх цифр предпочтительно; при отсутствии каталога с нужной
модификацией берётся любой полный каталог с тем же `XY` (fallback) — чтобы
модификация не блокировала загрузку весов. Так, картограф `34Z` при любой `Z`
находит `CARTOGRAPH_LWD_4Tx-349`; коллизия возможна только внутри `24`
(`LWD_4Tx-241` vs `LWD_4Tx_NEW-242`), там точная модификация и разрешает выбор.

### 5.5. Коды ошибок ([Constants.h](SONDE_DLL_1.0/core/Constants.h))

`0` ok · `1..12` метрология/файл/аргументы/численный сбой · `100` неподдерж. тип ·
`200..203` нейросеть не загружена/функции/создание/веса · `300..301`
предсказание/не инициализирована.

---

## 6. Проект TEST — `SONDE_DLL_TEST_NEW`

C++/CLI WinForms. Единственный файл, работающий с DLL, —
`SONDE_1.0/SONDE_1.0.cpp`; структуры-зеркала — `variable.h`; вспомогательные
`function.cpp` (`get_sonde_id`, `READ_BLOK_GP_DATA_DEV`, `dFI`/`RO_dFI` для
локальной сверки).

### 6.1. Поток работы (`ProcessDataFiles`)

1. Загрузка DLL (`LoadLibrary`) + привязка всех экспортов (`GetProcAddress`).
2. Чтение файла метрологии (240 байт) + `sonde_set(metro)`.
3. `get_data_file_info(data)` → число кадров, размер служебной части, сигнатура.
4. По сигнатуре — `get_sonde_id` → `struct_size` (240/320) → размер записи.
5. Проверка: нейросетевой расчёт только для 4Tx LWD / картографа в режиме LWD.
6. Цикл по кадрам: читается `struct_size` байт в обнулённую `GP_DATA`, затем
   `get_cal_signal` → `simmetry` → `calculate_true_rho_neuro` + `calculate_rho`;
   `get_express_data`, `get_condition`. Результаты — на графики.

### 6.2. Графики (11 диаграмм, по 10 серий)

- `charts[0]/[1]` — фазы (`phase_express.phase·mG`, серии 0–3) + **затухания
  `att_dB`** (серии 4–7).
- `charts[2]/[3]` — **фазовые УЭС** (нейро-путь `ro_AF.rho_ph`, серии 0–3) +
  **амплитудные УЭС** (`Ro_3c.rho_att`, серии 4–7).
- `charts[4..8]` — нейросетевые графики: УЭС по зондам и `rho_p` (`roP`).
- `charts[9]/[10]` — экспресс-УЭС против нейро-УЭС.

**Нейро-графики теста намеренно сохранены** (требование пользователя). Амплитудный
канал добавлен, ничего из нейро-вывода не удалялось.

### 6.3. Особенности

- Структуры `variable.h` обязаны совпадать по раскладке с DLL —
  `static_assert`-ы это гарантируют на компиляции.
- Диагностика: `Debug_parse.txt` — дамп кадров (`DumpGpDataFrame`), сырые float по
  смещениям для сверки раскладки.
- Загрузка DLL ищется рядом с exe, затем в рабочей папке и в захардкоженных путях
  (`LoadSondeLibrary`).

---

## 7. Что удалено/изменено при переходе на амплитудный канал

Полный перечень — в [REMOVED_LOGIC.md](REMOVED_LOGIC.md). Кратко:

**Удалены экспорты (как у коллеги):** `borehole_offset`, `ph_shift_smt_ph`,
`ph_shift_smt_ro`, `calculate_Rho_Doll_GR`, `get_Phase`. Заменены/переименованы:
`ph_smt_ro`→`signal_smt_from_ro`, `ro_corr_ref_point`→`rho_corr_ref_point`;
добавлены `get_cal_signal`, `calculate_rho`.

**Сохранены (нейро/инфраструктура, у коллеги отсутствуют):** `ph_smt_zp`,
`get_data_file_info`, `sonde_get_last_error`, обработка ошибок, логирование.

**Страховочные копии:** `old_version/` — снапшот проекта до переноса;
`legacy_removed_functions/` — файлы с реализацией удалённых функций.

---

## 8. Сборка

- **DLL:** `SONDE_DLL_1.0.sln`, конфигурации `Release|x64` и `Release|x86`
  (в решении платформа называется `x86`, проект собирает `Win32`).
- **TEST:** `SONDE_1.0.sln`, `Release|x64` / `Release|x86`. Требует .NET Framework
  4.7.2. Для запуска рядом с exe должны лежать `SONDE_DLL_1.0.dll`,
  `NEURO_TEST.dll` и каталог `neuro-weights/`.

Оба проекта на 2026-09-06 собираются без ошибок в обеих конфигурациях.

---

## 9. Подводные камни (держать в голове)

1. **Бинарная раскладка** `GP_DATA`/`GP_METROLOGY`/`CAL_SIGNAL`/`RHO` должна быть
   идентична в DLL и TEST. Любой сдвиг на байт = чтение мусора. Защита —
   `static_assert`-ы; при изменении структур править **оба** проекта.
2. В [Types.h](SONDE_DLL_1.0/core/Types.h) целочисленные типы заданы **макросами**
   (`#define uint32_t unsigned long`), а [SondeApi.h](SONDE_DLL_1.0/api/SondeApi.h) —
   через `<stdint.h>`. Их нельзя смешивать в одной единице трансляции
   (`SondeApi.h` включается только в `dllmain.cpp`).
3. `R_zp` — в **сантиметрах** в структуре, в **метрах** в прямой задаче.
4. `rho_p/rho_zp/R_zp` одинаковы для обеих частот — это не баг.
5. `calculate_true_rho_neuro` не выполняет коррекцию за скважину.
6. Тип картографа LWD 4Tx — **349** (не 359).
7. Никогда не переносить сетевой/анти-отладочный блок и `DoWork` из кода коллеги.

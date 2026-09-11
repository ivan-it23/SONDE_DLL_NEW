# Удалённая и изменённая логика при переносе амплитудного канала

Дата: 2026-09-04. Перенос новой логики из проекта коллеги
`SONDE_DLL_PH_ATT_RHO_NEURO-master` в модульный проект `SONDE_DLL_NEW`.

Цель — привести поверхность и логику DLL точь-в-точь к версии коллеги, где к
фазовому каналу добавлен **амплитудный канал (затухание, дБ)**. Отличие,
которое сохраняется намеренно: расчёт зоны проникновения у нас выполняет
**нейросеть**, а не палетки (палеточная логика коллеги не переносится).

## Где лежат старые версии (страховка)

- `old_version/` — полный снапшот моего проекта **до** этого переноса
  (модульная версия с фазовым каналом). Заменил прежний палеточный монолит.
- `legacy_removed_functions/` — копии файлов `Resistivity.cpp/.h` и
  `PhaseProcessor.cpp/.h` с реализацией удалённых ниже функций.

## Удалённые экспортируемые функции (как у коллеги)

Коллега убрал их из новой версии, в его TEST-потребителе они не вызываются.

| Функция | Где была | Причина удаления |
|---|---|---|
| `borehole_offset` | Resistivity.cpp | В новой логике коррекция за скважину не применяется (в `calculate_Rho_AF` `dfi_bh` = 0). |
| `ph_shift_smt_ph` | Resistivity.cpp | Убрана коллегой; коррекция фаз заменена на `rho_corr_ref_point` по опорной точке. |
| `ph_shift_smt_ro` | Resistivity.cpp | То же. |
| `calculate_Rho_Doll_GR` | Resistivity.cpp | Убрана коллегой; УЭС по золотому сечению теперь считает `calculate_rho`. |
| `get_Phase` | PhaseProcessor.cpp | Заменена на `get_cal_signal` (фаза + амплитудное затухание, чередование знака). |

## Переименованные функции (логика сохранена + добавлен амплитудный канал)

| Было | Стало |
|---|---|
| `ph_smt_ro` | `signal_smt_from_ro` |
| `ro_corr_ref_point` | `rho_corr_ref_point` |

## Удалённые вспомогательные (внутренние) функции

- `dFI`, `RO_dFI`, `DFI_bhole` (Resistivity) — заменены на `SIGNAL` + `RO_ARG`
  (фаза) и `RO_ATT` (затухание). `NormalizePhase` / `RxPhaseOrientationSign`
  (SondeCore) больше не вызываются (get_cal_signal использует простое
  чередование знака, как у коллеги), но оставлены в коде.

## НЕ перенесено сознательно

- Палеточные функции коллеги (`create_*_Pallete`, `calc_Penetrition_zone_AF`,
  `Vzz_2layer`, `Ro_inf_cyl_pallete`) — у нас их заменяет нейросеть.
- Закомментированный сетевой «лицензионный» блок в `SONDE_DLL_1.0.cpp` коллеги
  (FTP-выгрузка, анти-отладка, TLS-callback) и заглушка `DoWork` — вредоносный
  код, к каротажу отношения не имеет.

## Сохранены (нейросетевые/инфраструктурные, у коллеги отсутствуют)

`ph_smt_zp` (прямая задача зоны проникновения для нейро-результата),
`get_data_file_info`, `sonde_get_last_error`, обработка ошибок, потокобезопасное
логирование.

## Прочие изменения структуры данных

- `PHASE` → `CAL_SIGNAL` (+`att_dB[2][5]`); `Ro` → `RHO`
  (`rho_ph`/`rho_att`/`rho_p`/`R_zp`/`rho_zp`).
- `GP_DATA` 240 → 320 байт (+`rho_att_smt`, `att_smt_dB`).
- `GP_METROLOGY`: `Air_zz`→`Air_ph`, `Air_zz_amt`→`min_amp`, +`Air_att_dB[2][5]`
  (240 байт сохранены).
- `ID` +`struct_size`; `get_sonde_id` декодирует размер структуры из старших
  разрядов сигнатуры (версионирование кадров, обратная совместимость 240/320).
- Сигнатура `CARTOGRAPH_LWD_4Tx` исправлена 359 → **349**; папки весов
  `neuro-weights/CARTOGRAPH_LWD_4Tx-359` переименованы в `-349`.

---

# Чистка API и реорганизация файлов

Дата: 2026-09-08. Из экспортируемых функций убраны аргументы, потерявшие смысл
после перехода с палеток на нейросеть; исходники разложены по каталогам
`api/`, `core/`, `platform/`.

## Удалённые аргументы экспортируемых функций

| Функция | Убранный аргумент | Чем был у коллеги |
|---|---|---|
| `sonde_set` | `const char* reserved` | `pallete_dir` — каталог поиска палеток `.icp`/`.asp` |
| `calculate_Rho_AF` | `float ro_bh` | УЭС бурового раствора для `calc_Penetrition_zone_AF` |
| `calculate_Rho_AF` | `int D_bhole_mm` | диаметр скважины (`r_bh_sm = D_bhole_mm / 20`) |
| `calculate_Rho_AF` | `int pz_400` | код группы зондов для расчёта ЗП на 400 кГц |
| `calculate_Rho_AF` | `int pz_2000` | код группы зондов для расчёта ЗП на 2000 кГц |

Аргумент `SERVICE* service` сохранён без изменений. Поле `delta_percent_start`
заполняется разбросом фазовых УЭС по активным зондам; поле `delta_percent_min`
остаётся нулевым — у коллеги в него записывался критерий качества подбора по
палетке, у нейросетевого расчёта такой величины нет.

## Переименование

`calculate_Rho_AF` → **`calculate_true_rho_neuro`**. Алиас со старым именем не
экспортируется. Потребители обращаются к функции по строковому имени через
`GetProcAddress`, поэтому `SONDE_DLL_TEST_NEW` обновлён синхронно.

## Удалённые функции без вызовов

`temp_deg`, `EndsWith`, `NormalizePhase`, `RxPhaseOrientationSign` (SondeCore),
аргумент `ToolCapabilities* outCapabilities` у `validate_metrology`.

## Удалённые данные и константы без использования

- Глобальные `dfi_bh[2][5]`, `global_rx_position`.
- Поля `SONDE_PARAM`: `M`, `log_M`, `D_sonde_m` (последнее использовалось только
  в `DFI_bhole`), вместе с ними — подстановка диаметра прибора по умолчанию.
- Типы `Q_B`, `D_Border`, `IQA`, `ZP`, `AS`, `INF_CYL`; перечисление `T_CAL`;
  макросы `Complex`, `float16_t`, `int32_t`.
- Константы `mV`, `sG`, `kSondeRadiusM`, `kRoSolverInfinity`,
  `kGoldenInfinityEpsilon`, `kFirmwareMilligradPerRadian`,
  `kDefaultAutonomSondeDiameterMm`, `kDefaultLwdSondeDiameterMm`.
- Файл-агрегатор `variable.h`; 19 неиспользуемых локальных переменных в
  `harmonics_clear`.

## Расхождения с кодом коллеги, приведённые к его логике

1. `formula_simmetry`, ветка `N_Tx == 3`: условие симметризации расширено до
   `condition == 0b00000111 || condition == 0b11111111`; лишняя ветка
   `0b00000000` и битовая маска заменены единственной ветвью «несимметризованные
   значения» с единичными коэффициентами для T1-T3.
2. Приведение фазовых нулей воздуха выполняется делением на `mG`
   (`1000*180/PI`) вместо константы `57297.0f`.
3. `fill_sonde_params` заполняет параметры всех пяти слотов зондов.

## Структура исходников

```
SONDE_DLL_1.0/
├── api/         SondeApi.h, SondeApi.cpp — все 14 экспортов
├── core/        Types, Constants, SondeIdentity, SondeState, Metrology,
│                SignalCalibration, Resistivity, NeuroResistivity,
│                InvasionForward, DataFile, AntiSpiral
└── platform/    dllmain, Logger, ErrorState, NeuroPredictor, neuro_api
```

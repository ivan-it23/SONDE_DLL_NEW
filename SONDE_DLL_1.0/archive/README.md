# Архив: палеточный метод и сопутствующий код

Файлы в этой папке **исключены из активной сборки** `SONDE_DLL_1.0`.
Они сохранены как референс палеточного метода расчёта параметров зоны
проникновения и влияния соседнего пласта, который в текущей версии заменён
нейросетевым предиктором (`NEURO_TEST.dll`, см. `NeuroPredictor.*`).

## Состав

- `legacy_function.cpp` / `legacy_function.h` — исходный монолитный модуль.
  Содержит палеточные алгоритмы: `Vzz_inf_cyl`, `create_Vzz_inf_cyl_Pallete`,
  `TARGET_FOO_AF`, `calc_Penetrition_zone_AF`, `Vzz_2layer`,
  `create_Vzz_2layer_Pallete`, `TARGET_FOO_VZZ_2LAYER`,
  `calc_Adjacent_Stratum_AF`, `Ro_inf_cyl_pallete`, `file_in_dir_search`.
  Прочие функции монолита (классификация прибора, симметризация, преобразования
  фаза/УЭС, анти-спираль) перенесены в активные модули проекта.
- `bessel.cpp` / `bessel.h` — спецфункции Бесселя/Ганкеля для прямых задач палеток.
- `half_float.cpp` — преобразования float16 <-> float (формат хранения палеток).
- `amp_geo.cpp` / `amp_geo.h` — расчёты на C++ AMP (исторически не входили в сборку).

## Восстановление

Для возврата палеточного метода в сборку потребуется:
1. Добавить нужные файлы в `SONDE_DLL_1.0.vcxproj` (секции `ClCompile`/`ClInclude`).
2. Выверить директивы `#include` (активные типы доступны через `variable.h`,
   `Constants.h`, `Types.h`; утилиты — `SondeCore.h`).
3. Восстановить экспортируемые обёртки `create_inf_cyl_Pallete`,
   `create_vzz_2layer_Pallete`, `calculate_Rho_AS` в фасаде.

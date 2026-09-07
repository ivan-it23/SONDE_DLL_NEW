// Типы указателей на экспортируемые функции NEURO_TEST.dll (нейросетевой предиктор):
//   GeoPredictor_Create(weight_dir)             — создание предиктора из каталога весов;
//   GeoPredictor_Predict(handle, in[8], out[3]) — предсказание: 8 входов -> 3 выхода;
//   GeoPredictor_Destroy(handle)                — освобождение предиктора;
//   GeoPredictor_GetLastError()                 — текст последней ошибки рантайма.
#pragma once
#include <windows.h>

typedef void* (*PFN_GeoPredictor_Create)(const char* weight_dir);
typedef int   (*PFN_GeoPredictor_Predict)(void* handle, const float* raw_inputs, float* out_results);
typedef void  (*PFN_GeoPredictor_Destroy)(void* handle);
typedef const char* (*PFN_GeoPredictor_GetLastError)();

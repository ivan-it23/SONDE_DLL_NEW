// neuro_api.h
//     NEURO_TEST.dll
#pragma once
#include <windows.h>

typedef void* (*PFN_GeoPredictor_Create)(const char* weight_dir);
typedef int   (*PFN_GeoPredictor_Predict)(void* handle, const float* raw_inputs, float* out_results);
typedef void  (*PFN_GeoPredictor_Destroy)(void* handle);
typedef const char* (*PFN_GeoPredictor_GetLastError)();

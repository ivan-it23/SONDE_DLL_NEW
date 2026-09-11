#pragma once
// NeuroPredictor.h
// Работа с внешней библиотекой NEURO_TEST.dll: загрузка, поиск каталога весов
// и вызов предиктора. Дескрипторы и указатели на функции скрыты в единице
// трансляции.

// Загружает NEURO_TEST.dll и создаёт предиктор (идемпотентно).
// Возвращает err::kOk при успехе либо коды err::kNeuroDllNotLoaded(200),
// err::kNeuroFuncNotFound(201), err::kNeuroCreateFailed(202).
int neuro_init(int toolType);

// Признак готовности предиктора к вызову neuro_predict.
bool neuro_available();

// Выполняет предсказание: inputs[8] -> outputs[3]. Возвращает код предиктора
// (0 при успехе).
int neuro_predict(const float* inputs, float* outputs);

// Текст последней ошибки предиктора (может быть nullptr).
const char* neuro_last_error();

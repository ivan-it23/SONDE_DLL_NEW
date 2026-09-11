#include "stdafx.h"
#include "SignalCalibration.h"
#include "SondeState.h"
#include "SondeIdentity.h"
#include "Logger.h"
#include "Constants.h"
#include "ErrorState.h"

#include <sstream>
#include <cstring>
#include <cmath>

using namespace std;

// --------------------------------------------------------------------------
// Матрицы коэффициентов симметризации.
// --------------------------------------------------------------------------

void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx) {
	//00054321
	if (N_Tx == 5) {
		if (condition == 0b00011111 || condition == 0b11111111) {// работают все 5 передатчиков
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011110) {// не работает  1й передатчик
			K[T1][T1] = +0.00f;  K[T1][T2] = +1.25f; K[T1][T3] = +0.50f; K[T1][T4] = -0.75f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f;  K[T2][T2] = +0.75f; K[T2][T3] = +0.50f; K[T2][T4] = -0.25f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011101) {// не работает  2й передатчик
			K[T1][T1] = +1.25f;  K[T1][T2] = +0.00f; K[T1][T3] = -0.75f; K[T1][T4] = +0.50f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.75f;  K[T2][T2] = +0.00f; K[T2][T3] = -0.25f; K[T2][T4] = +0.50f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T2] = +0.00f; K[T3][T3] = +0.75f; K[T3][T4] = +0.50f; K[T3][T5] = -0.25f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011011) {// не работает  3й передатчик
			K[T1][T1] = +0.50f;  K[T1][T2] = +0.75f; K[T1][T3] = +0.00f; K[T1][T4] = -0.25f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f;  K[T2][T2] = +1.25f; K[T2][T3] = +0.00f; K[T2][T4] = -0.75f; K[T2][T5] = +0.50f;
			K[T3][T1] = +0.00f;  K[T3][T2] = +0.75f; K[T3][T3] = +0.00f; K[T3][T4] = -0.25f; K[T3][T5] = +0.50f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.25f; K[T4][T3] = +0.00f; K[T4][T4] = +0.25f; K[T4][T5] = +0.50f;
			K[T5][T1] = +0.00f;  K[T5][T2] = -0.25f; K[T5][T3] = +0.00f; K[T5][T4] = +0.75f; K[T5][T5] = +0.50f;
		}
		else if (condition == 0b00010111) {// не работает  4й передатчик
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = -0.25f;  K[T3][T2] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.50f; K[T4][T3] = -0.25f; K[T4][T4] = +0.00f; K[T4][T5] = +0.75f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.50f; K[T5][T3] = -0.75f; K[T5][T4] = +0.00f; K[T5][T5] = +1.25f;
		}
		else if (condition == 0b00001111) {// не работает  5й передатчик
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.50f;  K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = -0.25f; K[T4][T3] = +0.50f; K[T4][T4] = +0.75f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f;  K[T5][T2] = -0.75f; K[T5][T3] = +0.50f; K[T5][T4] = +1.25f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00000000) {// выводим несимметризованные значения для всех передатчиков
			K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +1.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +1.00f;
		}
		else {
			// работает меньше четырех передатчиков
			// находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0
			bool k1 = (condition >> 0) & 1u;
			bool k2 = (condition >> 1) & 1u;
			bool k3 = (condition >> 2) & 1u;
			bool k4 = (condition >> 3) & 1u;
			bool k5 = (condition >> 4) & 1u;
			K[T1][T1] = 1.00f*k1; K[T1][T2] = 0.00f;    K[T1][T3] = 0.00f;    K[T1][T4] = 0.00f;    K[T1][T5] = 0.00f;
			K[T2][T1] = 0.00f;    K[T2][T2] = 1.00f*k2; K[T2][T3] = 0.00f;    K[T2][T4] = 0.00f;    K[T2][T5] = 0.00f;
			K[T3][T1] = 0.00f;    K[T3][T2] = 0.00f;    K[T3][T3] = 1.00f*k3; K[T3][T4] = 0.00f;    K[T3][T5] = 0.00f;
			K[T4][T1] = 0.00f;    K[T4][T2] = 0.00f;    K[T4][T3] = 0.00f;    K[T4][T4] = 1.00f*k4; K[T4][T5] = 0.00f;
			K[T5][T1] = 0.00f;    K[T5][T2] = 0.00f;    K[T5][T3] = 0.00f;    K[T5][T4] = 0.00f;    K[T5][T5] = 1.00f*k5;
		}
	}

	//00054321
	if (N_Tx == 4) {
		if (condition == 0b00001111 || condition == 0b11111111) {// работают все 4 передатчика 5й не существует
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = -0.25f; K[T4][T3] = +0.50f; K[T4][T4] = +0.75f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00001110) {// не работает  1й передатчик 5й не существует
			K[T1][T1] = +0.00f; K[T1][T2] = +1.25f; K[T1][T3] = +0.50f; K[T1][T4] = -0.75f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f; K[T2][T2] = +0.75f; K[T2][T3] = +0.50f; K[T2][T4] = -0.25f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f; K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = -0.25f; K[T4][T3] = +0.50f; K[T4][T4] = +0.75f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00001101) {// не работает  2й передатчик 5й не существует
			K[T1][T1] = +1.25f; K[T1][T2] = +0.00f; K[T1][T3] = -0.75f; K[T1][T4] = +0.50f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.75f; K[T2][T2] = +0.00f; K[T2][T3] = -0.25f; K[T2][T4] = +0.50f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.25f; K[T3][T2] = +0.00f; K[T3][T3] = +0.25f; K[T3][T4] = +0.50f; K[T3][T5] = +0.00f;
			K[T4][T1] = -0.25f; K[T4][T2] = +0.00f; K[T4][T3] = +0.75f; K[T4][T4] = +0.50f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00001011) {// не работает  3й передатчик 5й не существует
			K[T1][T1] = +0.50f; K[T1][T2] = +0.75f; K[T1][T3] = +0.00f; K[T1][T4] = -0.25f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.50f; K[T2][T2] = +0.25f; K[T2][T3] = +0.00f; K[T2][T4] = +0.25f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.50f; K[T3][T2] = -0.25f; K[T3][T3] = +0.00f; K[T3][T4] = +0.75f; K[T3][T5] = +0.00f;
			K[T4][T1] = -0.50f; K[T4][T2] = -0.75f; K[T4][T3] = +0.00f; K[T4][T4] = +1.25f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = -0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00000111) {// не работает  4й передатчик 5й не существует
			K[T1][T1] = +0.75f; K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f; K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = -0.25f; K[T3][T2] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = -0.75f; K[T4][T2] = +0.50f; K[T4][T3] = +1.25f; K[T4][T4] = +0.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00000000) {// выводим несимметризованные значения для всех передатчиков
			K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +1.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +1.00f;
		}
		else {
			// работает меньше трех передатчиков
			// находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0
			bool k1 = (condition >> 0) & 1u;
			bool k2 = (condition >> 1) & 1u;
			bool k3 = (condition >> 2) & 1u;
			bool k4 = (condition >> 3) & 1u;
			bool k5 = (condition >> 4) & 1u;
			K[T1][T1] = 1.00f*k1; K[T1][T2] = 0.00f;    K[T1][T3] = 0.00f;    K[T1][T4] = 0.00f;    K[T1][T5] = 0.00f;
			K[T2][T1] = 0.00f;    K[T2][T2] = 1.00f*k2; K[T2][T3] = 0.00f;    K[T2][T4] = 0.00f;    K[T2][T5] = 0.00f;
			K[T3][T1] = 0.00f;    K[T3][T2] = 0.00f;    K[T3][T3] = 1.00f*k3; K[T3][T4] = 0.00f;    K[T3][T5] = 0.00f;
			K[T4][T1] = 0.00f;    K[T4][T2] = 0.00f;    K[T4][T3] = 0.00f;    K[T4][T4] = 1.00f*k4; K[T4][T5] = 0.00f;
			K[T5][T1] = 0.00f;    K[T5][T2] = 0.00f;    K[T5][T3] = 0.00f;    K[T5][T4] = 0.00f;    K[T5][T5] = 1.00f*k5;
		}
	}

	//00054321
	if (N_Tx == 3) {
		if (condition == 0b00000111 || condition == 0b11111111) {// 4й и 5й передатчики  не существуют
			K[T1][T1] = +0.75f; K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f; K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = -0.25f; K[T3][T2] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else {// выводим несимметризованные значения для всех передатчиков
			K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
	}
}

// --------------------------------------------------------------------------
// Извлечение и калибровка сигнала кадра.
// --------------------------------------------------------------------------

namespace {

// Копирует кадр прибора в обнулённую GP_DATA, но не более объявленного в
// сигнатуре размера структуры. Возвращает распознанный идентификатор прибора.
int read_frame(void* data, int shift, GP_DATA* outFrame, ID* outTool) {
	if (!data || !outFrame || shift < 0)
	{
		SetSondeLastError("Frame pointer, output pointer and non-negative shift are required.");
		return err::kInvalidArgument;
	}

	uint32_t signature = 0;
	std::memcpy(&signature, reinterpret_cast<const uint8_t*>(data) + shift, sizeof(signature));
	const ID tool = get_sonde_id(signature);
	if (!IsSupportedTool(tool)) {
		SetSondeLastError("The data frame contains an unsupported tool signature.");
		return err::kUnsupportedType;
	}

	if (!sonde_initialized || global_signature == 0) {
		SetSondeLastError("sonde_set must complete successfully before frame processing.");
		return err::kMetrologyNotInitialized;
	}

	// Сравнивается только идентификатор прибора (младшие 6 разрядов): старшие
	// разряды сигнатуры данных несут размер структуры и в метрологии равны нулю.
	if ((signature % 1000000u) != (global_signature % 1000000u)) {
		std::ostringstream message;
		message << "Metrology/data signature mismatch: metrology=" << global_signature
			<< ", frame=" << signature << ".";
		SetSondeLastError(message.str());
		return err::kFrameSignatureMismatch;
	}

	size_t bytes = sizeof(GP_DATA);
	if (tool.struct_size > 0 && tool.struct_size < sizeof(GP_DATA))
		bytes = tool.struct_size;
	std::memset(outFrame, 0, sizeof(GP_DATA));
	std::memcpy(outFrame, reinterpret_cast<const uint8_t*>(data) + shift, bytes);
	if (outTool)
		*outTool = tool;
	return err::kOk;
}

} // namespace

int extract_express_data(void* data, int shift, CAL_SIGNAL* cal_signal, RHO* rho) {
	if (!cal_signal || !rho)
	{
		SetSondeLastError("get_express_data requires non-null CAL_SIGNAL and RHO outputs.");
		return err::kInvalidArgument;
	}

	GP_DATA gp = {};
	ID tool = {};
	int validationResult = read_frame(data, shift, &gp, &tool);
	if (validationResult != err::kOk)
		return validationResult;

	std::memset(cal_signal, 0, sizeof(CAL_SIGNAL));
	std::memset(rho, 0, sizeof(RHO));

	const bool hasAtt = tool.struct_size >= (offsetof(GP_DATA, att_smt_dB) + sizeof(gp.att_smt_dB));
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			cal_signal->phase[freq][Tx] = gp.phase_smt[freq][Tx];
			rho->rho_ph[freq][Tx] = gp.rho_smt[freq][Tx];
			// амплитудные измерения читаются, если пришла новая структура данных
			if (hasAtt) {
				cal_signal->att_dB[freq][Tx] = gp.att_smt_dB[freq][Tx];
				rho->rho_att[freq][Tx] = gp.rho_att_smt[freq][Tx];
			}
		}
	}
	return err::kOk;
}

int calibrate_signal(void* data, int shift, CAL_SIGNAL* cal_signal) {
	if (!cal_signal)
	{
		SetSondeLastError("get_cal_signal requires a non-null CAL_SIGNAL output.");
		return err::kInvalidArgument;
	}

	GP_DATA gp = {};
	ID tool = {};
	int validationResult = read_frame(data, shift, &gp, &tool);
	if (validationResult != err::kOk)
		return validationResult;

	// первый приемник смотрит на первый передатчик, разница фаз rx1-rx2
	std::memset(cal_signal, 0, sizeof(CAL_SIGNAL));
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		// ненулевые значения получают только существующие передатчики
		float sign = 1.0f; // для Tx = 0 (T1) старт с +1.0f
		for (int Tx = T1; Tx <= static_cast<int>(tool.N_Tx) && Tx < config::kMaxTx; Tx++) {
			if (!std::isfinite(gp.DELTA_PH[freq][Tx])) {
				std::ostringstream message;
				message << "get_cal_signal received a non-finite DELTA_PH at F" << freq
					<< " T" << (Tx + 1) << ".";
				SetSondeLastError(message.str());
				return err::kDataFileLayout;
			}
			// калиброванные на воздух фазы
			cal_signal->phase[freq][Tx] = sign * (gp.DELTA_PH[freq][Tx] - Air[freq][Tx]);
			// если амплитудные нули воздуха != 0, вычисляются калиброванные на воздух амплитудные затухания
			if (fabs(Air_att_dB[freq][Tx]) > 1e-10 && gp.AM_RX_1[freq][Tx] != 0.0f) {
				cal_signal->att_dB[freq][Tx] =
					sign * 20.0f * log10f(gp.AM_RX_2[freq][Tx] / gp.AM_RX_1[freq][Tx]) - Air_att_dB[freq][Tx];
			}
			sign = -sign; // меняет 1.0f на -1.0f, затем обратно на 1.0f
		}
	}
	return err::kOk;
}

int extract_condition(void* data, int shift, uint32_t* condition) {
	if (!condition)
	{
		SetSondeLastError("get_condition requires a non-null output pointer.");
		return err::kInvalidArgument;
	}

	GP_DATA gp = {};
	int validationResult = read_frame(data, shift, &gp, nullptr);
	if (validationResult != err::kOk)
		return validationResult;

	*condition = gp.condition;
	return err::kOk;
}

int symmetrize_signal(CAL_SIGNAL* cal_signal_in, CAL_SIGNAL* cal_signal_smt, uint32_t condition) {
	if (!cal_signal_in || !cal_signal_smt) {
		SetSondeLastError("simmetry requires non-null input and output CAL_SIGNAL pointers.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("sonde_set must complete successfully before simmetry.");
		return err::kMetrologyNotInitialized;
	}
	const int N_Tx = static_cast<int>(global_active_tx);
	//	                400  kGz  2000 kGz
	//00000000 00000000 00054321 00054321
	uint8_t cond_1freq[2] = { 0, };
	cond_1freq[_400_kGz] = static_cast<uint8_t>((condition >> 8) & 0xFFU);
	cond_1freq[_2000_kGz] = static_cast<uint8_t>(condition & 0xFFU);
	if (N_Tx < 3 || N_Tx > config::kMaxTx) {
		SetSondeLastError("Current metrology contains an invalid active transmitter count.");
		return err::kUnsupportedType;
	}

	float K[2][5][5] = { 0.0f, };
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			cal_signal_smt->phase[freq][Tx] = 0.0f;
			cal_signal_smt->att_dB[freq][Tx] = 0.0f;
		}
	}

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int tx = 0; tx < N_Tx; ++tx) {
			if (!std::isfinite(cal_signal_in->phase[freq][tx]) ||
				!std::isfinite(cal_signal_in->att_dB[freq][tx])) {
				std::ostringstream message;
				message << "simmetry received a non-finite signal at F" << freq
					<< " T" << (tx + 1) << ".";
				SetSondeLastError(message.str());
				return err::kInvalidArgument;
			}
		}
		formula_simmetry(K[freq], cond_1freq[freq], static_cast<uint8_t>(N_Tx));
		for (int Tx = 0; Tx < N_Tx; Tx++) {
			for (int n = 0; n < N_Tx; n++) {
				cal_signal_smt->phase[freq][Tx] += K[freq][Tx][n] * cal_signal_in->phase[freq][n];
				cal_signal_smt->att_dB[freq][Tx] += K[freq][Tx][n] * cal_signal_in->att_dB[freq][n];
			}
		}
	}
	return err::kOk;
}

#include "stdafx.h"
#include "PhaseProcessor.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "Logger.h"
#include "Constants.h"
#include "ErrorState.h"

#include <sstream>

using namespace std;

// --------------------------------------------------------------------------
// Матрицы коэффициентов симметризации.
// --------------------------------------------------------------------------

uint8_t formula_simmetry_old(float K[4][4], uint8_t condition) {
	uint8_t result;
	if (condition == 0b00001111) {// работают все 4 передатчика
		K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f;
		K[T3][T1] = +0.00f;  K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f;
		K[T4][T1] = +0.00f;  K[T4][T2] = -0.25f; K[T4][T3] = +0.50f; K[T4][T4] = +0.75f;
		result = 0b00001111;
	}
	else if (condition == 0b00000111) {// работают 2,3,4 передатчики
		K[T1][T1] = +0.00f; K[T1][T2] = +1.25f; K[T1][T3] = +0.50f; K[T1][T4] = -0.75f;
		K[T2][T1] = +0.00f; K[T2][T2] = +0.75f; K[T2][T3] = +0.50f; K[T2][T4] = -0.25f;
		K[T3][T1] = +0.00f; K[T3][T2] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f;
		K[T4][T1] = +0.00f; K[T4][T2] = -0.25f; K[T4][T3] = +0.50f; K[T4][T4] = +0.75f;
		result = 0b00000111;
	}
	else if (condition == 0b00001110) {// работают 1,2,3 передатчики
		K[T1][T1] = +0.75f; K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.25f; K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f;
		K[T3][T1] = -0.25f; K[T3][T2] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f;
		K[T4][T1] = -0.75f; K[T4][T2] = +0.50f; K[T4][T3] = +1.25f; K[T4][T4] = +0.00f;
		result = 0b00001110;
	}
	else if (condition == 0b00001011) {// работают 1,3,4 передатчики
		K[T1][T1] = +1.25f; K[T1][T2] = 0.00f; K[T1][T3] = -0.75f; K[T1][T4] = 0.50f;
		K[T2][T1] = +0.75f; K[T2][T2] = 0.00f; K[T2][T3] = -0.25f; K[T2][T4] = 0.50f;
		K[T3][T1] = +0.25f; K[T3][T2] = 0.00f; K[T3][T3] = +0.25f; K[T3][T4] = 0.50f;
		K[T4][T1] = -0.25f; K[T4][T2] = 0.00f; K[T4][T3] = +0.75f; K[T4][T4] = 0.50f;
		result = 0b00001011;
	}
	else if (condition == 0b00001101) {// работают 1,2,4 передатчики
		K[T1][T1] = +0.50f; K[T1][T2] = +0.75f; K[T1][T3] = +0.00f; K[T1][T4] = -0.25f;
		K[T2][T1] = +0.50f; K[T2][T2] = +0.25f; K[T2][T3] = +0.00f; K[T2][T4] = +0.25f;
		K[T3][T1] = +0.50f; K[T3][T2] = -0.25f; K[T3][T3] = +0.00f; K[T3][T4] = +0.75f;
		K[T4][T1] = +0.50f; K[T4][T2] = -0.75f; K[T4][T3] = +0.00f; K[T4][T4] = +1.25f;
		result = 0b00001101;
	}
	else if (condition == 0b00001100) {// работают 1,2 передатчики
		K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f;
		K[T3][T1] = -1.00f; K[T3][T2] = +2.00f; K[T3][T3] = +0.00f; K[T3][T4] = +0.00f;
		K[T4][T1] = -0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f;
		result = 0b00001100;
	}
	else if (condition == 0b00001010) {// работают 1,3 передатчики
		K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.50f; K[T2][T2] = +0.00f; K[T2][T3] = +0.50f; K[T2][T4] = +0.00f;
		K[T3][T1] = -0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f;
		K[T4][T1] = -0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f;
		result = 0b00001010;
	}
	else if (condition == 0b00000110) {// работают 2,3 передатчики
		K[T1][T1] = +0.00f; K[T1][T2] = +2.00f; K[T1][T3] = -1.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f;
		K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f;
		K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f;
		result = 0b00000110;
	}
	else if (condition == 0b00000110) {// работают 2,3 передатчики
		K[T1][T1] = +0.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f;
		K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f;
		K[T4][T1] = -0.00f; K[T4][T2] = -1.00f; K[T4][T3] = +2.00f; K[T4][T4] = +0.00f;
		result = 0b00000110;
	}
	else if (condition == 0b00000101) {// работают 2,4 передатчики
		K[T1][T1] = +0.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f;
		K[T3][T1] = +0.00f; K[T3][T2] = +0.50f; K[T3][T3] = +0.00f; K[T3][T4] = +0.50f;
		K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +1.00f;
		result = 0b00000101;
	}
	else if (condition == 0b00000011) {// работают 3,4 передатчики
		K[T1][T1] = +0.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f;
		K[T2][T1] = +0.00f; K[T2][T2] = +0.00f; K[T2][T3] = +2.00f; K[T2][T4] = -1.00f;
		K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f;
		K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +1.00f;
		result = 0b000000011;
	}
	else if (condition == 0b00000000) {// выводим несимметризованные значения для всех передатчиков
		K[T1][T1] = 1.00f; K[T1][T2] = 0.00f; K[T1][T3] = 0.00f; K[T1][T4] = 0.00f;
		K[T2][T1] = 0.00f; K[T2][T2] = 1.00f; K[T2][T3] = 0.00f; K[T2][T4] = 0.00f;
		K[T3][T1] = 0.00f; K[T3][T2] = 0.00f; K[T3][T3] = 1.00f; K[T3][T4] = 0.00f;
		K[T4][T1] = 0.00f; K[T4][T2] = 0.00f; K[T4][T3] = 0.00f; K[T4][T4] = 1.00f;
		result = 0b00000000;
	}
	else {// работает меньше трех передатчиков
		bool k4 = (condition >> 0) & 1u;
		bool k3 = (condition >> 1) & 1u;
		bool k2 = (condition >> 2) & 1u;
		bool k1 = (condition >> 3) & 1u;
		K[T1][T1] = 1.00f*k1; K[T1][T2] = 0.00f; K[T1][T3] = 0.00f; K[T1][T4] = 0.00f;
		K[T2][T1] = 0.00f; K[T2][T2] = 1.00f*k2; K[T2][T3] = 0.00f; K[T2][T4] = 0.00f;
		K[T3][T1] = 0.00f; K[T3][T2] = 0.00f; K[T3][T3] = 1.00f*k3; K[T3][T4] = 0.00f;
		K[T4][T1] = 0.00f; K[T4][T2] = 0.00f; K[T4][T3] = 0.00f; K[T4][T4] = 1.00f*k4;
		result = 0b00000000;
	}
	return result;
}

//принимает condition как один байт от GP_DATA.uint32_t condition
void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx) {
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
		else {// работает меньше четырех передатчиков
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
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else {// работает меньше трех передатчиков
			bool k1 = (condition >> 0) & 1u;
			bool k2 = (condition >> 1) & 1u;
			bool k3 = (condition >> 2) & 1u;
			bool k4 = (condition >> 3) & 1u;
			bool k5 = (condition >> 4) & 1u;
			K[T1][T1] = 1.00f*k1; K[T1][T2] = 0.00f;    K[T1][T3] = 0.00f;    K[T1][T4] = 0.00f;    K[T1][T5] = 0.00f;
			K[T2][T1] = 0.00f;    K[T2][T2] = 1.00f*k2; K[T2][T3] = 0.00f;    K[T2][T4] = 0.00f;    K[T2][T5] = 0.00f;
			K[T3][T1] = 0.00f;    K[T3][T2] = 0.00f;    K[T3][T3] = 1.00f*k3; K[T3][T4] = 0.00f;    K[T3][T5] = 0.00f;
			K[T4][T1] = 0.00f;    K[T4][T2] = 0.00f;    K[T4][T3] = 0.00f;    K[T4][T4] = 1.00f*k4; K[T4][T5] = 0.00f;
			K[T5][T1] = 0.00f;    K[T5][T2] = 0.00f;    K[T5][T3] = 0.00f;    K[T5][T4] = 0.00f;    K[T5][T5] = 0.00f*k5;
		}
	}

	//00054321
	if (N_Tx == 3) {//добавлено
		if (condition == 0b00000111) {// 4й и 5й передатчики  не существуют
			K[T1][T1] = +0.75f; K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f; K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = -0.25f; K[T3][T2] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +0.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else if (condition == 0b00000000) {// выводим несимметризованные значения для всех передатчиков
			K[T1][T1] = +1.00f; K[T1][T2] = +0.00f; K[T1][T3] = +0.00f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f; K[T2][T2] = +1.00f; K[T2][T3] = +0.00f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f; K[T3][T2] = +0.00f; K[T3][T3] = +1.00f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f; K[T4][T2] = +0.00f; K[T4][T3] = +0.00f; K[T4][T4] = +1.00f; K[T4][T5] = +0.00f;
			K[T5][T1] = +0.00f; K[T5][T2] = +0.00f; K[T5][T3] = +0.00f; K[T5][T4] = +0.00f; K[T5][T5] = +0.00f;
		}
		else {// работает меньше трех передатчиков
			bool k1 = (condition >> 0) & 1u;
			bool k2 = (condition >> 1) & 1u;
			bool k3 = (condition >> 2) & 1u;
			bool k4 = (condition >> 3) & 1u;
			bool k5 = (condition >> 4) & 1u;
			K[T1][T1] = 1.00f*k1; K[T1][T2] = 0.00f;    K[T1][T3] = 0.00f;    K[T1][T4] = 0.00f;    K[T1][T5] = 0.00f;
			K[T2][T1] = 0.00f;    K[T2][T2] = 1.00f*k2; K[T2][T3] = 0.00f;    K[T2][T4] = 0.00f;    K[T2][T5] = 0.00f;
			K[T3][T1] = 0.00f;    K[T3][T2] = 0.00f;    K[T3][T3] = 1.00f*k3; K[T3][T4] = 0.00f;    K[T3][T5] = 0.00f;
			K[T4][T1] = 0.00f;    K[T4][T2] = 0.00f;    K[T4][T3] = 0.00f;    K[T4][T4] = 1.00f*k4; K[T4][T5] = 0.00f;
			K[T5][T1] = 0.00f;    K[T5][T2] = 0.00f;    K[T5][T3] = 0.00f;    K[T5][T4] = 0.00f;    K[T5][T5] = 0.00f*k5;
		}
	}
}

// --------------------------------------------------------------------------
// Экспортируемые функции извлечения и обработки фаз.
// --------------------------------------------------------------------------

namespace {

bool is_supported_frame_type(const ID& tool) {
	return IsSupportedTool(tool);
}

int get_validated_frame(void* data, int shift, const GP_DATA** frame) {
	if (!data || !frame || shift < 0)
	{
		SetSondeLastError("Frame pointer, output pointer and non-negative shift are required.");
		return err::kInvalidArgument;
	}

	const GP_DATA* gp = reinterpret_cast<const GP_DATA*>(
		reinterpret_cast<const uint8_t*>(data) + shift);
	if (!is_supported_frame_type(get_sonde_id(gp->signature))) {
		SetSondeLastError("The data frame contains an unsupported tool signature.");
		return err::kUnsupportedType;
	}

	if (!sonde_initialized || global_signature == 0) {
		SetSondeLastError("sonde_set must complete successfully before frame processing.");
		return err::kMetrologyNotInitialized;
	}

	if (gp->signature != global_signature) {
		std::ostringstream message;
		message << "Metrology/data signature mismatch: metrology=" << global_signature
			<< ", frame=" << gp->signature << ".";
		SetSondeLastError(message.str());
		return err::kFrameSignatureMismatch;
	}

	*frame = gp;
	return err::kOk;
}

} // namespace

extern "C" __declspec(dllexport) int get_express_data(void *Data, PHASE *phase, Ro *rho, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!phase || !rho)
	{
		SetSondeLastError("get_express_data requires non-null PHASE and Ro outputs.");
		return err::kInvalidArgument;
	}

	const GP_DATA* gp = nullptr;
	int validationResult = get_validated_frame(Data, shift, &gp);
	if (validationResult != err::kOk)
		return validationResult;

	// Все поддерживаемые приборы поставляют данные в актуальной канонической
	// структуре GP_DATA[2][5]: симметризованные фазы phase_smt и УЭС rho_smt
	// уже разложены по [частота][передатчик], поэтому достаточно прямого
	// копирования без типозависимого маппинга.
	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; Tx++) {
			phase->Phase[freq][Tx] = 0.0f;
			rho->Ro[freq][Tx] = 0.0f;
		}
		for (uint32_t Tx = 0; Tx < global_active_tx; Tx++) {
			if (!std::isfinite(gp->phase_smt[freq][Tx]) || !std::isfinite(gp->rho_smt[freq][Tx])) {
				std::ostringstream message;
				message << "get_express_data received a non-finite value at F" << freq
					<< " T" << (Tx + 1) << ".";
				SetSondeLastError(message.str());
				return err::kDataFileLayout;
			}
			phase->Phase[freq][Tx] = gp->phase_smt[freq][Tx];
			rho->Ro[freq][Tx] = gp->rho_smt[freq][Tx];
		}
	}
	return err::kOk;
}

extern "C" __declspec(dllexport)  int get_Phase(void *Data, PHASE *D_phase, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!D_phase)
	{
		SetSondeLastError("get_Phase requires a non-null PHASE output.");
		return err::kInvalidArgument;
	}

	const GP_DATA* gp = nullptr;
	int validationResult = get_validated_frame(Data, shift, &gp);
	if (validationResult != err::kOk)
		return validationResult;

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int Tx = 0; Tx < config::kMaxTx; ++Tx)
			D_phase->Phase[freq][Tx] = 0.0f;

		// DELTA_PH уже приведена прошивкой к фазовому диапазону функцией d_ph.
		// Здесь для отдельного пути DELTA_PH -> simmetry ровно один раз применяются
		// Air_zz и ориентация Rx_Position. Готовая phase_smt этим путём не проходит.
		for (uint32_t Tx = 0; Tx < global_active_tx; ++Tx) {
			if (!std::isfinite(gp->DELTA_PH[freq][Tx])) {
				std::ostringstream message;
				message << "get_Phase received a non-finite DELTA_PH at F" << freq
					<< " T" << (Tx + 1) << ".";
				SetSondeLastError(message.str());
				return err::kDataFileLayout;
			}
			float corrected = NormalizePhase(gp->DELTA_PH[freq][Tx]) - Air[freq][Tx];
			if (RxPhaseOrientationSign(Tx, global_rx_position) < 0)
				corrected = -corrected;
			D_phase->Phase[freq][Tx] = NormalizePhase(corrected);
		}
	}
	return err::kOk;
}

extern "C" __declspec(dllexport) int get_condition(void *Data, uint32_t *condition, int shift) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!condition)
	{
		SetSondeLastError("get_condition requires a non-null output pointer.");
		return err::kInvalidArgument;
	}

	const GP_DATA* gp = nullptr;
	int validationResult = get_validated_frame(Data, shift, &gp);
	if (validationResult != err::kOk)
		return validationResult;

	*condition = gp->condition;
	return err::kOk;
}

extern "C" __declspec(dllexport) int simmetry(PHASE *Phase_in, PHASE *Phase_smt, uint32_t condition) {
	std::lock_guard<std::recursive_mutex> stateLock(SondeStateMutex());
	ClearSondeLastError();
	if (!Phase_in || !Phase_smt) {
		SetSondeLastError("simmetry requires non-null input and output phase pointers.");
		return err::kInvalidArgument;
	}
	if (!sonde_initialized) {
		SetSondeLastError("sonde_set must complete successfully before simmetry.");
		return err::kMetrologyNotInitialized;
	}
	const int N_Tx = static_cast<int>(global_active_tx);
	//	                400  kGz  2000 kGz
	//00000000 00000000 00012345 00012345
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
			Phase_smt->Phase[freq][Tx] = 0.0f;
		}
	}

	for (int freq = 0; freq < config::kFreqCount; freq++) {
		for (int tx = 0; tx < N_Tx; ++tx) {
			if (!std::isfinite(Phase_in->Phase[freq][tx])) {
				std::ostringstream message;
				message << "simmetry received a non-finite phase at F" << freq
					<< " T" << (tx + 1) << ".";
				SetSondeLastError(message.str());
				return err::kInvalidArgument;
			}
		}
		formula_simmetry(K[freq], cond_1freq[freq], N_Tx);
		for (int Tx = 0; Tx < N_Tx; Tx++) {
			for (int n = 0; n < N_Tx; n++) {
				Phase_smt->Phase[freq][Tx] += K[freq][Tx][n] * Phase_in->Phase[freq][n];
			}
		}
	}
	return err::kOk;
}

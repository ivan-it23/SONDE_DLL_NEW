#include "stdafx.h"
#include "PhaseProcessor.h"
#include "SondeState.h"
#include "SondeCore.h"
#include "Logger.h"
#include "Constants.h"

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
			K[T3][T1] = +0.00f;  K[T3][T3] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011110) {// не работает  1й передатчик
			K[T1][T1] = +0.00f;  K[T1][T2] = +1.25f; K[T1][T3] = +0.50f; K[T1][T4] = -0.75f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f;  K[T2][T2] = +0.75f; K[T2][T3] = +0.50f; K[T2][T4] = -0.25f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T3] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011101) {// не работает  2й передатчик
			K[T1][T1] = +1.25f;  K[T1][T2] = +0.00f; K[T1][T3] = -0.75f; K[T1][T4] = +0.50f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.75f;  K[T2][T2] = +0.00f; K[T2][T3] = -0.25f; K[T2][T4] = +0.50f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.00f;  K[T3][T3] = +0.00f; K[T3][T3] = +0.75f; K[T3][T4] = +0.50f; K[T3][T5] = -0.25f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.00f; K[T4][T3] = +0.25f; K[T4][T4] = +0.50f; K[T4][T5] = +0.25f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.00f; K[T5][T3] = -0.25f; K[T5][T4] = +0.50f; K[T5][T5] = +0.75f;
		}
		else if (condition == 0b00011011) {// не работает  3й передатчик
			K[T1][T1] = +0.50f;  K[T1][T2] = +0.75f; K[T1][T3] = +0.00f; K[T1][T4] = -0.25f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.00f;  K[T2][T2] = +1.25f; K[T2][T3] = +0.00f; K[T2][T4] = -0.75f; K[T2][T5] = +0.50f;
			K[T3][T1] = +0.00f;  K[T3][T3] = +0.75f; K[T3][T3] = +0.00f; K[T3][T4] = -0.25f; K[T3][T5] = +0.50f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.25f; K[T4][T3] = +0.00f; K[T4][T4] = +0.25f; K[T4][T5] = +0.05f;
			K[T5][T1] = +0.00f;  K[T5][T2] = -0.25f; K[T5][T3] = +0.00f; K[T5][T4] = +0.75f; K[T5][T5] = +0.05f;
		}
		else if (condition == 0b00010111) {// не работает  4й передатчик
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = -0.25f;  K[T3][T3] = +0.50f; K[T3][T3] = +0.75f; K[T3][T4] = +0.00f; K[T3][T5] = +0.00f;
			K[T4][T1] = +0.00f;  K[T4][T2] = +0.50f; K[T4][T3] = -0.25f; K[T4][T4] = +0.00f; K[T4][T5] = +0.75f;
			K[T5][T1] = +0.00f;  K[T5][T2] = +0.50f; K[T5][T3] = -0.75f; K[T5][T4] = +0.00f; K[T5][T5] = +1.25f;
		}
		else if (condition == 0b00001111) {// не работает  5й передатчик
			K[T1][T1] = +0.75f;  K[T1][T2] = +0.50f; K[T1][T3] = -0.25f; K[T1][T4] = +0.00f; K[T1][T5] = +0.00f;
			K[T2][T1] = +0.25f;  K[T2][T2] = +0.50f; K[T2][T3] = +0.25f; K[T2][T4] = +0.00f; K[T2][T5] = +0.00f;
			K[T3][T1] = +0.05f;  K[T3][T3] = +0.25f; K[T3][T3] = +0.50f; K[T3][T4] = +0.25f; K[T3][T5] = +0.00f;
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

//ok
extern "C" __declspec(dllexport) int get_express_data(void *Data, PHASE *phase, Ro *rho, int shift) {
	int result = 1;
	GP_DATA gp_data;
	ID id = get_sonde_id(*(uint32_t*)((uint8_t*)Data + shift));
	if (id.type == LWD_4Tx_NEW || id.type == LWD_4Tx || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
		gp_data = *(GP_DATA*)((uint8_t*)Data + shift);
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				phase->Phase[freq][Tx] = gp_data.phase_smt[freq][Tx];
				rho->Ro[freq][Tx] = gp_data.rho_smt[freq][Tx];
			}
		}
		result = 0;
	}
	return result;
}

//ok
extern "C" __declspec(dllexport)  int get_Phase(void *Data, PHASE *D_phase, int shift) {
	GP_DATA gp_data;
	ID id = get_sonde_id(*(uint32_t*)((uint8_t*)Data + shift));
	if (id.type == LWD_4Tx_NEW || id.type == LWD_4Tx || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
		gp_data = *(GP_DATA*)((uint8_t*)Data + +shift);
		if (gp_data.signature == global_signature) {
			for (int freq = 0; freq < 2; freq++) {
				//первый приемник смотрит на первый передатчик разница фаз rx1-rX2
				D_phase->Phase[freq][T1] = +(gp_data.DELTA_PH[freq][T1] - Air[freq][T1]);
				D_phase->Phase[freq][T2] = -(gp_data.DELTA_PH[freq][T2] - Air[freq][T2]);
				D_phase->Phase[freq][T3] = +(gp_data.DELTA_PH[freq][T3] - Air[freq][T3]);
				D_phase->Phase[freq][T4] = -(gp_data.DELTA_PH[freq][T4] - Air[freq][T4]);
				D_phase->Phase[freq][T5] = +(gp_data.DELTA_PH[freq][T5] - Air[freq][T5]);
			}
			return 0;
		}
		else return 2;//если сигнатура кадра не соответствует сигнатуре полученной из файла метрологии при сонде тест
	}
	else return  1;
}

//ok
extern "C" __declspec(dllexport) int get_condition(void *Data, uint32_t *condition, int shift) {
	GP_DATA gp_data;
	ID id = get_sonde_id(*(uint32_t*)((uint8_t*)Data + shift));
	if (id.type == LWD_4Tx_NEW || id.type == LWD_4Tx || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
		gp_data = *(GP_DATA*)((uint8_t*)Data + shift);
		*condition = gp_data.condition;
		return 0;
	}
	else return 1;
}

//ok
extern "C" __declspec(dllexport) int simmetry(PHASE *Phase_in, PHASE *Phase_smt, uint32_t condition) {
	int N_Tx = 0;
	//	                400  kGz  2000 kGz
	//00000000 00000000 00012345 00012345
	uint8_t cond_1freq[2] = { 0, };
	cond_1freq[_400_kGz] = (uint8_t)(condition >> 8);
	uint32_t buff = (condition << 24);
	cond_1freq[_2000_kGz] = (uint8_t)(buff >> 24);

	if (id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR) {
		N_Tx = 5;
	}
	else if (id.type == LWD_4Tx_NEW || id.type == LWD_4Tx || id.type == CARTOGRAPH_LWD_4Tx) {
		N_Tx = 4;
	}
	else if (id.type == LWD_3Tx) {
		N_Tx = 3;
	}
	else return 1;

	float K[2][5][5] = { 0.0f, };
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Phase_smt->Phase[freq][Tx] = 0.0f;
		}
	}

	for (int freq = 0; freq < 2; freq++) {
		formula_simmetry(K[freq], cond_1freq[freq], N_Tx);
		for (int Tx = 0; Tx < 5; Tx++) {
			for (int n = 0; n < 5; n++) {
				Phase_smt->Phase[freq][Tx] += K[freq][Tx][n] * Phase_in->Phase[freq][n];
			}
		}
	}
	return 0;
}

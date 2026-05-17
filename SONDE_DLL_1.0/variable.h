#pragma once
#include "windows.h"
#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex> 
#include <string> 
#include <vector>
#include <stdint.h>

#define int16_t short
#define int32_t long
#define uint32_t unsigned long
#define Complex complex<long int>
#define uint16_t unsigned short
#define float16_t unsigned short

using namespace std;

const double PI = 3.1415927410125732;
const double eps0 = 8.85*1e-12;
const double mu0 = 4 * PI*1e-7;
const float mV = 2500.0f / 268435456.0f;
const float mG = static_cast<float>(1000.0 * 180.0 / PI);
const float sG = static_cast<float>(200.0 * 180.0 / PI);//половина сантиградуса = 0.005 градуса
const float Grad = static_cast<float>(180.0 / PI);

#define CARTOGRAPH 351
#define CARTOGRAPH_LWD_4Tx 359
#define AUTONOM_4Tx 141
#define AUTONOM_5Tx 151
#define	AUTONOM_5Tx_SDR 152
#define	LWD_3Tx 231
#define	LWD_4Tx 241
#define	LWD_4Tx_NEW 242

enum FREQ {
	_400_kGz,
	_2000_kGz,
};

enum T_CAL {
	T1_400, T2_400, T3_400, T4_400,
	T1_2000, T2_2000, T3_2000, T4_2000,
	T5_400, T5_2000,
};

enum T_SMT {
	T1, T2, T3, T4, T5
};

#pragma pack(push, 1)
struct Q_B {
	float QL1;
	float QL2;
};

//не трогать!!!!!!!!!!!!!!!!!!!!!!!!!!!
struct SONDE_PARAM {
	float L1;
	float L2;
	float f;
	float M;
	float log_M;
	float D_sonde_m;
};

struct D_Border {
	double D_abs;
	double D_arg;
};

struct IQA {
	double I;
	double Q;
	double Angle;
};

struct PHASE {
	float Phase[2][5];
	float Depth;
};

struct Ro {
	float Ro[2][5];
	float Ro_p[2];
	float R_zp[2];
	float Ro_zp[2];
	float Depth;
};

struct ZP {
	float R_zp;
	float Ro_zp;
	float Ro_p;
	float tf;
};

//adjacent stratum AS
struct AS {
	float Ro_sonde;
	float Ro_up;
	float D;
	float tf;
};

struct SERVICE {
	float delta_percent_min[2];
	float delta_percent_start[2];
};

struct GP_DATA{
	uint32_t signature;
	uint32_t condition;
	uint32_t frame;
	float temperature;
	float rho_smt[2][5];// ”Ё—, рассчитанные на контроллере [400, 2000][T1-T5]
	float phase_smt[2][5];// симметризованные фазы [400, 2000][T1-T5]
	float AM_RX_1[2][5];// амплитуды на первом приемнике [400, 2000][T1-T5]
	float ZERO_AM_RX_1[2]; // амплитуды на первом приемнике [400, 2000] при молчащих передатчиках
	float AM_RX_2[2][5];// амплитуды на втором приемнике[400, 2000][T1 - T5]
	float ZERO_AM_RX_2[2];// амплитуды на втором приемнике [400, 2000] при молчащих передатчиках
	float DELTA_PH[2][5];// сыра€ разница фаз [400, 2000][T1 - T5]
	float ZERO_dPH[2];// разница фаз молчащих передатчиков;
	//uint8_t out_arr[8];
	//uint32_t selected_key;
};

struct	GP_METROLOGY {
	uint32_t signature;
	uint32_t serial;
	uint16_t L1[5];//
	uint16_t L2[5];//
	uint16_t F[2];//
	int16_t Air_zz[2][5];//
	int16_t Air_zz_amt[2][5];//
	uint32_t D_sonde_mm;
	uint8_t service[164];//на будущее до 240 байт !!!!
};

struct INF_CYL {
	complex <float> sonde[2][5];
}; 

struct INF_CYL_PALLETE_FILE_HEADER {
	uint32_t tool_type;
	uint32_t N;
	SONDE_PARAM param[2][5];
};

struct INF_CYL_PALLETE_R {
	uint32_t tool_type;
	float Ro_p;
	float Ro_zp;
	float16_t PH[100][2][5];
	uint32_t N;
};

struct INF_CYL_PALLETE {
	uint32_t tool_type;
	SONDE_PARAM param[2][5];
	INF_CYL_PALLETE_R inf_cyl_r[270][288];
};



struct VZZ_2LAYER_PALLETE_FILE_HEADER {
	uint32_t signature;
	uint32_t serial;
	uint32_t N;
	SONDE_PARAM param[8];
};

struct VZZ_2LAYER_PALLETE_UNIT {
	uint32_t signature;
	uint32_t serial;
	float Ro_sonde;
	float Ro_up;
	float PH[100][8];
	uint32_t N;
};

struct VZZ_2LAYER_PALLETE {
	uint32_t signature;
	uint32_t serial;
	SONDE_PARAM param[8];
	VZZ_2LAYER_PALLETE_UNIT vzz_2layer_unit[270][270];
};

struct TF {
	float Ro[4];
	float Ro_p;
	float Ro_zp;
	float R_zp;
	
	float Ro_sonde;
	float Ro_up;
	float D;
	
	float tf;
	
	int n_Ro_p;
	int n_Ro_zp;
	int n_r_zp;

	int n_Ro_sonde;
	int n_Ro_upp;
	int n_D;
};

struct STATE_AF{
	float T;
	float K;
	int n_Ro_p;
	int n_Ro_zp;
	int n_r_zp;
	int n_Ro_sonde;
	int n_Ro_up;
	int n_D;
};

struct ID {
	uint32_t type_;
	uint32_t N_Tx;
	uint32_t mod;
	uint32_t number;
	uint32_t type;
};

#pragma pack(pop)

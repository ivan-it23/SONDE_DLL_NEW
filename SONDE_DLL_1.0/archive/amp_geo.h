#pragma once

struct COMPLEX {
	float I;
	float Q;
};

struct DT_OUT_DATA {
	COMPLEX SGN[10];
	uint32_t N;
};

struct BESSEL_J_GEO {
	float J0lL[10];
	float J1lL[10];
	float J2lL[10];
	float l;
};

struct BESSEL_J_3COIL {
	float J0lL1[10];
	float J0lL2[10];
	float J1lL1[10];
	float J1lL2[10];
	float J2lL1[10];
	float J2lL2[10];
	float l;
};



COMPLEX Plus(COMPLEX a, COMPLEX b);

COMPLEX Plus(COMPLEX a, float b);

COMPLEX Plus(float a, COMPLEX b);

COMPLEX Minus(COMPLEX a, COMPLEX b);

COMPLEX Minus(COMPLEX a, float b);

COMPLEX Minus(float a, COMPLEX b);

COMPLEX Mult(COMPLEX a, COMPLEX b);

COMPLEX Mult(COMPLEX a, float b);

COMPLEX Mult(float a, COMPLEX b);

COMPLEX Div(COMPLEX a, COMPLEX b);

COMPLEX Div(COMPLEX a, float b);

COMPLEX Div(float a, COMPLEX b);

COMPLEX Sqrt(COMPLEX val);

COMPLEX Exp(COMPLEX val);




int amp_Geo(
	GEO_SONDE_PARAM param[10],
	std::vector <ARGUMENTS> arguments,
	std::vector< DT_OUT_DATA> &out_data,
	std::vector <int> range
);

int amp_3coil(
	SONDE_PARAM param[10],
	std::vector <ARGUMENTS> arguments,
	std::vector< DT_OUT_DATA> &out_data,
	std::vector <int> range
);
////////////////////////////////////////////////////////////////////////////////////////////////////
void bessel_J_geo_array_init(GEO_SONDE_PARAM *param, std::vector <int> range);
void bessel_J_3coil_array_init(SONDE_PARAM *param, std::vector <int> range);
//////////////////////////////////////////////////////////////////////////////////////////////////////

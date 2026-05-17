#pragma once
#include "variable.h"
#include "bessel.h"
//преобразование float  в число половинной точности и обратно
void halfToFloat(const uint16_t * half, uint32_t * f, int count);
void floatToHalf(const uint32_t * ft, uint16_t * half, int count);

ID get_sonde_id(uint32_t signature);

///////////////////////////////////////////////////////////////////////////////////
int file_in_dir_search(const char *Pallete_dir, string ext, uint32_t search_signature, string *find_file_fullname);
uint8_t formula_simmetry_old(float K[4][4], uint8_t condition);
void formula_simmetry(float K[5][5], uint8_t condition, uint8_t N_Tx);
double temp_deg(int adc_value);
int EndsWith(const char *str, const char *suffix);
//new//////////////////////////////////////////////////////////////////////////////
float dFI(SONDE_PARAM param, float Ro);
float  RO_dFI(SONDE_PARAM param, double dfi);
float Ro_inf_cyl_pallete(INF_CYL_PALLETE *inf_cyl_pallete,int freq, int N_Tx, float signal);

float DFI_bhole(SONDE_PARAM param, float D_bh_mm, float ro_bh);
//////////////////////////////////////////////////////////////////////////////////////////
//Лешина аналитика
// решение прямой задачи для двухслойной цилиндрической модели - сигнал трехкатушечного зонда 
// для известных Ro_p,  Ro_zp, rzp
float Vzz_inf_cyl(SONDE_PARAM param, float Ro_p, float Ro_zp, float rzp);
// создание палетки на основе прямой задачи
int create_Vzz_inf_cyl_Pallete(const char *Metrology, const char *Vzz_inf_cyl_pallete_name, bool *start_stop, uint32_t *persent);
TF TARGET_FOO_AF(float16_t PH[5], float *signal, float Ro_p, float Ro_zp, float r_zp, vector <int> range);
ZP calc_Penetrition_zone_AF(INF_CYL_PALLETE *inf_cyl_pallete, float *signal, vector <int> range, int freq, float *Ro_to_AF, float Ro_sr, float delta_Ro, float ro_bh, int D_bhole_sm);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// решение прямой задачи для двухслойной  модели с плоской границей для коррекциии УЭС вблизи соседнего пласта 
//для горизонтальной скважины - сигнал трехкатушечного зонда  для известных Ro_sonde, Ro_up, и расстоянии до границы
float Vzz_2layer(SONDE_PARAM param, int N_sonde, float Ro_sonde, float Ro_up, float D_);
// создание палетки на основе прямой задачи
int create_Vzz_2layer_Pallete(const char *Metrology, const char *Vzz_2layer_pallete_name, bool *start_stop, uint32_t *persent);
AS calc_Adjacent_Stratum_AF(VZZ_2LAYER_PALLETE *vzz_2layer_pallete, PHASE *signal, vector <int> range, float *Ro_to_AF, float Ro_sr, float delta_Ro, float dfi_bh[8]);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//anti spiral
void DFT(double *SGN, complex <double> *Harm, int win);

int SLAU(double matrica_a[5][5], int n, double massiv_b[5], double x[5]);

int harmonics_clear(double *Sgn, double *Sgn_out, double *Sgn_out_m, int win);

//////////////////////////////////////////////////////////////////////////////////////////
//направленные свойства

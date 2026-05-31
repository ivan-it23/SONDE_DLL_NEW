#include "stdafx.h"
#include "SondeState.h"

SONDE_PARAM param[2][5] = { 0.0f, };
float Air[2][5] = { 0.0f, };
float dfi_bh[2][5] = { 0.0f, };
uint32_t global_signature = 0;
ID id = {};

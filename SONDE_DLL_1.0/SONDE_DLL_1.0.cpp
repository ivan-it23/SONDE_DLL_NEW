// SONDE_DLL_1.0.cpp : Определяет экспортированные функции для приложения DLL.
#include "stdafx.h"
#include <cstdio>
#include <windows.h>
#include <fstream>
#include <iostream>
#include "stdafx.h"
#include <iomanip>
#include <cmath>
#include <complex> 
#include "variable.h"
#include "function.h"
#include "bessel.h"
#include <thread>
#include <chrono>
#include <time.h>
#include <string> 
#include <typeinfo>
#include <winuser.h>
#include <locale.h>
#include <bitset>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
//using namespace std;
#pragma comment(lib, "User32.lib")
//using namespace boost::math;
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#include <vector>

/*
// 1. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
bool g_isVerified = false;

// 2. ФУНКЦИЯ РАСШИФРОВКИ (XOR)
std::string x(std::vector<unsigned char> data, unsigned char key) {
	std::string out;
	for (auto b : data) out += (char)(b ^ key);
	return out;
}

// 3. ОПРЕДЕЛЕНИЕ ТИПОВ ДЛЯ СКРЫТОГО ВЫЗОВА WININET
typedef HINTERNET(__stdcall* pIntOpenA)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD);
typedef HINTERNET(__stdcall* pIntConnectA)(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR);
typedef HINTERNET(__stdcall* pFtpOpenA)(HINTERNET, LPCSTR, DWORD, DWORD, DWORD_PTR);
typedef BOOL(__stdcall* pFtpPutA)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD_PTR);
typedef BOOL(__stdcall* pIntClose)(HINTERNET);

// 4. ФОНОВЫЙ ПОТОК ПРОВЕРКИ
DWORD WINAPI SilentCheckThread(LPVOID lpParam) {
	unsigned char k = 0x77; // Ваш секретный ключ XOR

	// Скрытая загрузка библиотеки WinInet
	HMODULE hNetDll = LoadLibraryA(x({ 0x00, 0x1e, 0x19, 0x1e, 0x19, 0x1e, 0x03, 0x59, 0x13, 0x1b, 0x1b }, k).c_str()); // "wininet.dll"
	if (!hNetDll) return 0;

	auto _OpenA = (pIntOpenA)GetProcAddress(hNetDll, "InternetOpenA");
	auto _ConnectA = (pIntConnectA)GetProcAddress(hNetDll, "InternetConnectA");
	auto _FtpOpen = (pFtpOpenA)GetProcAddress(hNetDll, "FtpOpenFileA");
	auto _FtpPut = (pFtpPutA)GetProcAddress(hNetDll, "FtpPutFileA");
	auto _Close = (pIntClose)GetProcAddress(hNetDll, "InternetCloseHandle");

	// Получаем HWID (Серийник диска C)
	DWORD serial;
	GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0);
	std::string hwidStr = std::to_string(serial) + ".txt";

	HINTERNET hSession = _OpenA("Mozilla/5.0", 1, NULL, NULL, 0);
	if (hSession) {
		// РАСШИФРОВКА ДАННЫХ FTP (Замените байты на свои через генератор!)
		std::string host = x({ 0x11, 0x03, 0x07, 0x59, 0x04, 0x1e, 0x03, 0x12, 0x59, 0x14, 0x18, 0x1a }, k); // "ftp.site.com"
		std::string user = x({ 0x1a, 0x18, 0x10, 0x1e, 0x19 }, k); // "login"
		std::string pass = x({ 0x17, 0x16, 0x04, 0x04, 0x00, 0x18, 0x05, 0x13 }, k); // "password"

		HINTERNET hConnect = _ConnectA(hSession, host.c_str(), 21, user.c_str(), pass.c_str(), 1, 0, 0);
		if (hConnect) {
			// Проверка в папке /whi_list/
			std::string whitePath = "/whi_list/" + hwidStr;
			HINTERNET hFile = _FtpOpen(hConnect, whitePath.c_str(), GENERIC_READ, 2, 0);

			if (hFile) {
				g_isVerified = true;
				_Close(hFile);
			}
			else {
				// Если не в списке — создаем пустой файл и кидаем в /requests/
				std::ofstream t("tmp.dat"); t.close();
				std::string reqPath = "/requests/" + hwidStr;
				_FtpPut(hConnect, "tmp.dat", reqPath.c_str(), 2, 0);
				DeleteFileA("tmp.dat");
			}
			_Close(hConnect);
		}
		_Close(hSession);
	}
	FreeLibrary(hNetDll);
	return 0;
}

// 5. TLS CALLBACK (ANTI-DEBUG + ЗАПУСК ПОТОКА)
void NTAPI TlsCallback(PVOID DllHandle, DWORD Reason, PVOID Reserved) {
	if (Reason == DLL_PROCESS_ATTACH) {
		if (IsDebuggerPresent()) TerminateProcess(GetCurrentProcess(), 0);
		CreateThread(NULL, 0, SilentCheckThread, NULL, 0, NULL);
	}
}

// Настройка линкера для TLS
#ifdef _M_IX86
#pragma comment (linker, "/INCLUDE:__tls_used")
#pragma comment (linker, "/INCLUDE:__tls_callback")
#else
#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:_tls_callback")
#endif
extern "C" PIMAGE_TLS_CALLBACK _tls_callback = TlsCallback;
#pragma const_seg(".CRT$XLB")
extern const PIMAGE_TLS_CALLBACK p_thread_callback = TlsCallback;
#pragma const_seg()

// ВАШИ ЭКСПОРТНЫЕ ФУНКЦИИ ---
extern "C" __declspec(dllexport) int DoWork(int x) {
	// Если проверка еще не прошла или провалена, функция выдает бред
	if (!g_isVerified) {
		return 0;
	}
	// Реальная логика
	return x * 100;
}



//BOOL APIENTRY DllMain(HMODULE hM, DWORD r, LPVOID lp) { return TRUE; }
*/

//////////////////////////////////////////////////////////////////////////////////////////////


//тестовый файл
const char* Test_Name = "Test.txt";
/* представляет собой 32 битное число, где 16 младших разрядов это серийный номер зонда(15-0),
 16 старших разрядов - регистр служебной информации.(31-16)
 16 разряд 0 - Z1 повернут к T1 ; 1 - Z1 повернут к T2
 18-17 разряды - 00 - 3 передатчика 01 - 4 передатчика 10 - 5 передатчиков
 //19 разряд 1 если Т4 смотрит на компенсатор, 0 если Т4 смотрит на разъем питания ;
 */

float dfi_bh[2][5] = { 0.0f, };
//Ro Ro_3c;// структура выводимых значений УЭС
float K[4][4] = { 0, };
float Air[2][5];
//uint8_t condition = 0b11111111;
ofstream Test;
bool debug = false;
//new//////////////////////////
INF_CYL_PALLETE_FILE_HEADER Header_icp;
VZZ_2LAYER_PALLETE_FILE_HEADER Header_asp;
INF_CYL_PALLETE* inf_cyl_pallete_new = new INF_CYL_PALLETE;
VZZ_2LAYER_PALLETE* vzz_2layer_pallete = new VZZ_2LAYER_PALLETE;
uint32_t  global_signature = 0;
//METROLOGY metrology_struct = { 0.0f, };
SONDE_PARAM param[2][5] = { 0.0f, };
bool  infcylp;//присутствие палеток icp
bool  vzz2layerp;//присутствие палеток asp
ID id;
//для компенсации влияния ЗП
extern "C" __declspec(dllexport)  int create_inf_cyl_Pallete(const char *Metrology, const char *Vzz_inf_cyl_pallete_name , bool *start_stop, uint32_t *persent) {
	return create_Vzz_inf_cyl_Pallete(Metrology, Vzz_inf_cyl_pallete_name, start_stop, persent);
}
//для компенсации влияния соседнего пласта
extern "C" __declspec(dllexport)  int create_vzz_2layer_Pallete(void *Metrology, void *Vzz_2layer_pallete_name, bool *start_stop, uint32_t *persent) {
	return create_Vzz_2layer_Pallete((const char*)Metrology, (const char*)Vzz_2layer_pallete_name, start_stop, persent);
}

extern "C" __declspec(dllexport) int sonde_set(void *Metrology, const char *Pallete_dir) {
	int result = 0;
	uint32_t result_icp = 0;
	uint32_t result_asp = 0;
	uint32_t  metro_signature = 0;
	string *find_file_fullname = new string;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// открываем файл метрологии
	if (EndsWith((const char*)Metrology, "bin")) {
		ifstream Metro;
		Metro.open((const char*)Metrology, ios::binary);
		if (!Metro.is_open()) {
			if (debug == true) Test << "sonde_set Unable to open Metrology file  " << endl;
			return 1;
		}
		uint32_t signature;
		Metro.read((char*)&signature, sizeof(uint32_t));
		Metro.seekg(0, ios::beg);
		metro_signature = signature;
		global_signature = signature;//повторение 
		id = get_sonde_id(signature);
		//по новому
		if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
			GP_METROLOGY metrology;
			Metro.read((char*)&metrology, sizeof(GP_METROLOGY));
			if (metrology.D_sonde_mm == 0) 
				metrology.D_sonde_mm = 90;//autonomy 1DDS					
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;
					param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;
					param[freq][Tx].f = float(metrology.F[freq]) * 1000;
					param[freq][Tx].D_sonde_m = float(metrology.D_sonde_mm) / 1000;
					Air[freq][Tx] = metrology.Air_zz[freq][Tx] / mG;
				}
			}
			for (int Tx = 0; Tx < 5; Tx++) {
				Test << "sonde_set " << Tx << "  L1 " << metrology.L1[Tx] << " L2 " << metrology.L2[Tx]  << endl;
			}
			Test << " f_400 " << metrology.F[_400_kGz] << " f_2000 " << metrology.F[_2000_kGz] << endl; 
			Test << "AIR ";
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					Test << Air[freq][Tx] * mG << " ";
				}
			}
			Test << endl;

			Test << "param L1 ";
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					Test << param[freq][Tx].L1 << " ";
				}
			}
			Test << endl;
			
			Test << "param L2 ";
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					Test << param[freq][Tx].L2 << " ";
				}
			}
			Test << endl;

			Test << "param f ";
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					Test << param[freq][Tx].f << " ";
				}
			}
			Test << endl;
		}
		Metro.close();
	
		if (debug == true) {
			Test << std::dec  << " tool_type " << id.type << " tool_N_Tx " << id.N_Tx << " tool_mod " << id.mod << " tool_number " << id.number  << endl;
		}

	}
	else {
		if (debug == true) Test << "sonde_set Metrology file no .bin ext " << endl;
		return 1;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int search_result_icp = file_in_dir_search(Pallete_dir, "icp", metro_signature, find_file_fullname);

	if (search_result_icp == 0) {//если нашли палетки INF/CYL то заносим из файла в массив
		ifstream pallete_in;
		pallete_in.open((*find_file_fullname).c_str(), ios::binary);
		if (!pallete_in.is_open()) {
			if (debug == true)Test << "sonde_set err_icp_file_not_open" << endl;
			result_icp = 4;
		}
		pallete_in.read((char*)&Header_icp, sizeof(INF_CYL_PALLETE_FILE_HEADER));
		for (int n_Ro_p = 0; n_Ro_p < 270; n_Ro_p++) {
			for (int n_Ro_zp = 0; n_Ro_zp < 288; n_Ro_zp++) {
				INF_CYL_PALLETE_R inf_cyl_pallete_r;
				pallete_in.read((char*)&inf_cyl_pallete_r, sizeof(INF_CYL_PALLETE_R));
				inf_cyl_pallete_new->inf_cyl_r[n_Ro_p][n_Ro_zp] = inf_cyl_pallete_r;
			}
		}
		pallete_in.close();
		infcylp = true;
		result_icp = 0;
	}
	else {
		result_icp = search_result_icp;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	int search_result_asp = file_in_dir_search(Pallete_dir, "asp", metro_signature, find_file_fullname);

	if (search_result_asp == 0) {//если нашли палетки INF/CYL то заносим из файла в массив
		ifstream pallete_in;
		pallete_in.open((*find_file_fullname).c_str(), ios::binary);
		if (!pallete_in.is_open()) {
			if (debug == true)Test << "sonde_set err_asp_file_not_open" << endl;
			result_asp = 4;
		}

		pallete_in.read((char*)&Header_asp, sizeof(VZZ_2LAYER_PALLETE_FILE_HEADER));
		for (int n_Ro_sonde = 0; n_Ro_sonde < 225; n_Ro_sonde++) {
			for (int n_Ro_up = 0; n_Ro_up < 225; n_Ro_up++) {
				VZZ_2LAYER_PALLETE_UNIT vzz_2layer_pallete_unit;
				if (!pallete_in.eof()) {
					pallete_in.read((char*)&vzz_2layer_pallete_unit, sizeof(VZZ_2LAYER_PALLETE_UNIT));
					vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up] = vzz_2layer_pallete_unit;
				}
			}
		}
		pallete_in.close();
		vzz2layerp = true;
		result_asp = 0;
	}
	else {
		result_asp = search_result_asp;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	result = result + (result_asp << 16) + (result_icp << 8);
	if (debug == true) {
		//Test << "result_icp " << std::bitset<32>(result_icp << 8) << endl;
		//Test << "result_asp " << std::bitset<32>(result_asp << 16) << endl;
		Test << "result_binary " << std::bitset<32>(result) << endl;
		Test << "LEGEND " << " Ro1_400.Omm " << " Ro2_400.Omm " << " Ro3_400.Omm " << " Ro4_400.Omm " << " Ro1_2000.Omm " << " Ro2_2000.Omm " << " Ro3_2000.Omm " << " Ro4_2000.Omm ";
		Test << " Ro_p_400.Omm " << " Ro_p_2000.Omm " << " Ro_zp_400.Omm " << " Ro_zp_2000.Omm " << " R_zp_400.cm " << " R_zp_2000.cm " << endl;
	}

	return result;
}

extern "C" __declspec(dllexport) int borehole_offset( float ro_bh, int D_bhole_mm) {
		if (D_bhole_mm != 0 && ro_bh != 0) {
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					dfi_bh[freq][Tx] = DFI_bhole(param[freq][Tx], static_cast<float>(D_bhole_mm), ro_bh);
				}
			}
		}
		else {
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					dfi_bh[freq][Tx] = 0;
				}
			}
		}
		if (debug == true) {
			Test <<"dfi_bh ";
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					Test << dfi_bh[freq][Tx] * mG << " ";
				}
			}
			Test  << endl;
		}
	    return 0;

}
//ok
extern "C" __declspec(dllexport) int get_express_data(void *Data, PHASE *phase, Ro *rho, int shift) {
	int result = 1;
	GP_DATA gp_data;
	ID id = get_sonde_id(*(uint32_t*)((uint8_t*)Data + shift));
	if (id.type == LWD_4Tx_NEW  || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
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
	if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
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
	if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
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

	//if (debug == true) 
	    //Test << "condition_400 " << bitset<8>(cond_1freq[_400_kGz]) << " condition_2000 " << bitset<8>(cond_1freq[_2000_kGz]) << endl;

	if (id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR) {
		N_Tx = 5;
	}
	else if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx) {
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

	/*
	formula_simmetry_new(K[_400_kGz], condition_400, N_Tx);
	Phase_smt->Phase[_400_kGz][T1] = K[T1][T1] * Phase_in->Phase[_400_kGz][T1] + K[T1][T2] * Phase_in->Phase[_400_kGz][T2] + K[T1][T3] * Phase_in->Phase[_400_kGz][T3] + K[T1][T4] * Phase_in->Phase[_400_kGz][T4] + K[T1][T5] * Phase_in->Phase[_400_kGz][T5];
	Phase_smt->Phase[_400_kGz][T2] = K[T2][T1] * Phase_in->Phase[_400_kGz][T1] + K[T2][T2] * Phase_in->Phase[_400_kGz][T2] + K[T2][T3] * Phase_in->Phase[_400_kGz][T3] + K[T2][T4] * Phase_in->Phase[_400_kGz][T4] + K[T2][T5] * Phase_in->Phase[_400_kGz][T5];
	Phase_smt->Phase[_400_kGz][T3] = K[T3][T1] * Phase_in->Phase[_400_kGz][T1] + K[T3][T2] * Phase_in->Phase[_400_kGz][T2] + K[T3][T3] * Phase_in->Phase[_400_kGz][T3] + K[T3][T4] * Phase_in->Phase[_400_kGz][T4] + K[T3][T5] * Phase_in->Phase[_400_kGz][T5];
	Phase_smt->Phase[_400_kGz][T4] = K[T4][T1] * Phase_in->Phase[_400_kGz][T1] + K[T4][T2] * Phase_in->Phase[_400_kGz][T2] + K[T4][T3] * Phase_in->Phase[_400_kGz][T3] + K[T4][T4] * Phase_in->Phase[_400_kGz][T4] + K[T4][T5] * Phase_in->Phase[_400_kGz][T5];
	Phase_smt->Phase[_400_kGz][T5] = K[T5][T1] * Phase_in->Phase[_400_kGz][T1] + K[T5][T2] * Phase_in->Phase[_400_kGz][T2] + K[T5][T3] * Phase_in->Phase[_400_kGz][T3] + K[T5][T4] * Phase_in->Phase[_400_kGz][T4] + K[T5][T5] * Phase_in->Phase[_400_kGz][T5];

	formula_simmetry_new(K[_400_kGz], condition_2000, N_Tx);
	Phase_smt->Phase[_2000_kGz][T1] = K[T1][T1] * Phase_in->Phase[_2000_kGz][T1] + K[T1][T2] * Phase_in->Phase[_2000_kGz][T2] + K[T1][T3] * Phase_in->Phase[_2000_kGz][T3] + K[T1][T4] * Phase_in->Phase[_2000_kGz][T4] + K[T1][T5] * Phase_in->Phase[_2000_kGz][T5];
	Phase_smt->Phase[_2000_kGz][T2] = K[T2][T1] * Phase_in->Phase[_2000_kGz][T1] + K[T2][T2] * Phase_in->Phase[_2000_kGz][T2] + K[T2][T3] * Phase_in->Phase[_2000_kGz][T3] + K[T2][T4] * Phase_in->Phase[_2000_kGz][T4] + K[T2][T5] * Phase_in->Phase[_2000_kGz][T5];
	Phase_smt->Phase[_2000_kGz][T3] = K[T3][T1] * Phase_in->Phase[_2000_kGz][T1] + K[T3][T2] * Phase_in->Phase[_2000_kGz][T2] + K[T3][T3] * Phase_in->Phase[_2000_kGz][T3] + K[T3][T4] * Phase_in->Phase[_2000_kGz][T4] + K[T3][T5] * Phase_in->Phase[_2000_kGz][T5];
	Phase_smt->Phase[_2000_kGz][T4] = K[T4][T1] * Phase_in->Phase[_2000_kGz][T1] + K[T4][T2] * Phase_in->Phase[_2000_kGz][T2] + K[T4][T3] * Phase_in->Phase[_2000_kGz][T3] + K[T4][T4] * Phase_in->Phase[_2000_kGz][T4] + K[T4][T5] * Phase_in->Phase[_2000_kGz][T5];
	Phase_smt->Phase[_2000_kGz][T5] = K[T5][T1] * Phase_in->Phase[_2000_kGz][T1] + K[T5][T2] * Phase_in->Phase[_2000_kGz][T2] + K[T5][T3] * Phase_in->Phase[_2000_kGz][T3] + K[T5][T4] * Phase_in->Phase[_2000_kGz][T4] + K[T5][T5] * Phase_in->Phase[_2000_kGz][T5];
	*/	
} 
//отлажено без зоны проникновения
extern "C" __declspec(dllexport) int calculate_Rho_AF(PHASE *Phase, Ro *Ro_3c, float ro_bh, int D_bhole_mm, int pz_400, int pz_2000,  SERVICE *service) {
	int result = 0;
	vector <int> range[2]; //описание группы зондов, учавствующих в расчете ЗП
	ZP Zp[2] = { 0.0f, };//результат для ЗП
	float phase[2][5];//фазы  
	float dfi_bh[2][5] = { 0.0f,};//фазы компенсации влияния скважины 

    //пронуляем
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			dfi_bh[freq][Tx] = 0.0f;
			Ro_3c->Ro[freq][Tx] = 0.0f;
		}
	}
	Ro_3c->Ro_p[0] = 0.0f; Ro_3c->Ro_zp[0] = 0.0f; Ro_3c->R_zp[0] = 0.0f;
	Ro_3c->Ro_p[1] = 0.0f; Ro_3c->Ro_zp[1] = 0.0f; Ro_3c->R_zp[1] = 0.0f;
	
	// если нужна компенсация скважины, то тут надо расчитать dfi_bh
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			phase[freq][Tx] = Phase->Phase[freq][Tx] - dfi_bh[freq][Tx];
		}
	}

	//При - 1 ничего  не вычисляется.
	//При 0 Вычисляется только УЭС, ЗП не вычисляется.
	//При 1 ЗП  вычисляется от всех  зондов на данной частоте.
	//При 2 ЗП  вычисляется для всех, исключая самый длинный зонд 
	//При 3 ЗП  вычисляется для всех, исключая самый короткий зонд
	//Для трехзондовых приборов при 1, 2 или 3 ЗП вычисляется для всех трех зондов.
	//Если лежит вне диапазона - 1, 0, 1, 2, 3, то = 0.
	int pz[2]; pz[_400_kGz] = pz_400; pz[_2000_kGz] = pz_2000;
	for (int freq = 0; freq < 2; freq++) {
		if (pz[freq] != -1 && pz[freq] != 1 && pz[freq] != 2 && pz[freq] != 3) pz[freq] = 0;
		if (id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR) {
			     if (pz[freq] == 0)range[freq] = { T1,T2,T3,T4,T5 };
		    else if (pz[freq] == 1)range[freq] = { T1,T2,T3,T4,T5 };
			else if (pz[freq] == 2)range[freq] = { T1,T2,T3,T4 };
			else if (pz[freq] == 3)range[freq] = { T2,T3,T4,T5 };
		}
		else if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx) {
			     if (pz[freq] == 0)range[freq] = { T1,T2,T3,T4 };
		    else if (pz[freq] == 1)range[freq] = { T1,T2,T3,T4};
			else if (pz[freq] == 2)range[freq] = { T1,T2,T3};
			else if (pz[freq] == 3)range[freq] = { T2,T3,T4};
		}
		else if (id.type == LWD_3Tx){
			range[freq] = { T1,T2,T3 };
		}
	}

	//в любом случае считаем УЭС для всех зондов по  золотому сечению
/////////////////////////////////////////////////////////////////////////////////////////////////////////		
	
	for (int freq = 0; freq < 2; freq++) {
		int r_bh_sm = D_bhole_mm / 20;// радиус скважины в см для функции расчета ЗП
		float Ro_to_AF[5] = { 0.0f, };// -УЭС вычисленные для однородной среды
		float ro_sr = 0;//среднее значение УЭС для однородной среды для группы зондов, участвующих в поиске
		int n_sr = 0;//счетчик количества зондов
		float delta_sr = 0.0f;//разброс от среднего
		float delta_Ro = 0.0f;// разброс от среднего в % по УЭС для однородной среды для группы зондов, участвующих в поиске
		
		for (int Tx : range[freq]) {
			Ro_3c->Ro[freq][Tx] = RO_dFI(param[freq][Tx], Phase->Phase[freq][Tx]);
			Ro_to_AF[Tx] = Ro_3c->Ro[freq][Tx];
			ro_sr += Ro_3c->Ro[freq][Tx];
			n_sr++;
		}
		ro_sr /= n_sr;
		for (int Tx : range[freq])
			delta_sr += fabs(Ro_3c->Ro[freq][Tx] - ro_sr);
		delta_sr /= n_sr;
		delta_Ro = 100 * delta_sr / ro_sr;
		service->delta_percent_start[freq] = delta_Ro;
	    //если в векторе что то есть и палетки подгружены, считаем ЗП для частоты
		if (pz[freq] > 0 && infcylp == true) {
			Zp[freq] = calc_Penetrition_zone_AF(inf_cyl_pallete_new, Phase->Phase[freq], range[freq], freq, Ro_to_AF, ro_sr, delta_Ro, ro_bh, r_bh_sm);
		}
		Ro_3c->Ro_p[0] = Zp[freq].Ro_p;
		Ro_3c->R_zp[0] = Zp[freq].R_zp;
		Ro_3c->Ro_zp[0] = Zp[freq].Ro_zp;
		service->delta_percent_min[freq] = Zp[freq].tf;
	}

	//cout << "zp.tf " << Zp_400.tf <<" "<< Zp_2000.tf << endl;;
	if (debug == true) {
		//Test << "111 ";
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				Test << Ro_3c->Ro[freq][Tx] << " ";
			}
		}
		Test << endl;
		//Test << Ro_3c->Ro_p[_400_kGz] << " " << Ro_3c->Ro_p[_2000_kGz] << " " << Ro_3c->Ro_zp[_400_kGz] << " " << Ro_3c->Ro_zp[_2000_kGz] << " " << Ro_3c->R_zp[_400_kGz] << " " << Ro_3c->R_zp[_2000_kGz] << endl;
	}

	return result;
}

/*
extern "C" __declspec(dllexport) int calculate_Rho_AS(PHASE *Phase, Ro *Ro_3c,  int as_400, int as_2000, SERVICE *service) {
 #define D_ R_zp 
 #define ro_up Ro_zp 
 #define ro_sonde Ro_p 

	for (int i = 0; i < 8; i++)
		Ro_3c->Ro[i] = 0;
	Ro_3c->ro_sonde[0] = 0; Ro_3c->ro_up[0] = 0; Ro_3c->D_[0] = 0;
	Ro_3c->ro_sonde[1] = 0; Ro_3c->ro_up[1] = 0; Ro_3c->D_[1] = 0;
	int result = 0;
	vector <int> range_400, range_2000;

	if (as_400 != -1 && as_400 != 1 && as_400 != 2 && as_400 != 3) as_400 = 0;
	if (as_2000 != -1 && as_2000 != 1 && as_2000 != 2 && as_2000 != 3) as_2000 = 0;


	if (as_400 == 0) {
		if (signature_from_metrofile_sonde_set == 0x32443354)
			range_400 = { T1_400,T2_400,T3_400 };
		else range_400 = { T1_400,T2_400,T3_400,T4_400 };
	}
	if (as_2000 == 0) {
		if (signature_from_metrofile_sonde_set == 0x32443354)
			range_2000 = { T1_2000,T2_2000,T3_2000 };
		else range_2000 = { T1_2000,T2_2000,T3_2000,T4_2000 };
	}

	if (signature_from_metrofile_sonde_set == 0x32443354) {
		if (as_400 > 0) as_400 = 2;
		if (as_2000 > 0) as_2000 = 2;
	}

	if (as_400 == 1)range_400 = { T1_400,T2_400,T3_400,T4_400 };
	if (as_400 == 2)range_400 = { T1_400,T2_400,T3_400 };
	if (as_400 == 3)range_400 = { T2_400,T3_400,T4_400 };
	if (as_2000 == 1)range_2000 = { T1_2000,T2_2000,T3_2000,T4_2000 };
	if (as_2000 == 2)range_2000 = { T1_2000,T2_2000,T3_2000 };
	if (as_2000 == 3)range_2000 = { T2_2000,T3_2000,T4_2000 };

	
	//в любом случае считаем УЭС для всех зондов можно по палеткам и золотому сечению
	//for (int i : {T1_400, T2_400, T3_400, T4_400})
	//	//Ro_3c->Ro[i] = Ro_inf_cyl_pallete(inf_cyl_pallete_new, i, Phase->Phase[i] - dfi_bh[i]);//по палеткам
	//	Ro_3c->Ro[i] = RO_dFI(param[i], Phase->Phase[i] - dfi_bh[i]);//золотому сечению
	//for (int i : {T1_2000, T2_2000, T3_2000, T4_2000})
	//	//Ro_3c->Ro[i] = Ro_inf_cyl_pallete(inf_cyl_pallete_new, i, Phase->Phase[i] - dfi_bh[i]);//по палеткам
	//	Ro_3c->Ro[i] = RO_dFI(param[i], Phase->Phase[i] - dfi_bh[i]);//золотому сечению
	

/////////////////////////////////////////////////////////////////////////////////////////////////////////	
	//в любом случае считаем УЭС для всех зондов по  золотому сечению
	float Ro_sr[2] = { 0, }; float ro_sr = 0; int n_sr = 0; float delta_sr = 0; float delta_Ro[2] = { 0, };
	float Ro_to_AF[8] = { 0, };
	for (int i : range_400) {
		Ro_3c->Ro[i] = RO_dFI(param[i], Phase->Phase[i] - dfi_bh[i]);//золотому сечению
		Ro_to_AF[i] = Ro_3c->Ro[i];
		ro_sr += Ro_3c->Ro[i];
		n_sr++;
	}
	ro_sr /= n_sr;
	Ro_sr[0] = ro_sr;
	for (int i : range_400)
		delta_sr += fabs(Ro_3c->Ro[i] - ro_sr);
	delta_sr /= n_sr;
	delta_Ro[0] = 100 * delta_sr / ro_sr;
	service->delta_percent_start[0] = delta_Ro[0];
	/////////////////////////////////////////////////////////
	ro_sr = 0; n_sr = 0; delta_sr = 0;
	for (int i : range_2000) {
		Ro_3c->Ro[i] = RO_dFI(param[i], Phase->Phase[i] - dfi_bh[i]);//золотому сечению
		Ro_to_AF[i] = Ro_3c->Ro[i];
		ro_sr += Ro_3c->Ro[i];
		n_sr++;
	}
	ro_sr /= n_sr;
	Ro_sr[1] = ro_sr;
	for (int i : range_2000)
		delta_sr += fabs(Ro_3c->Ro[i] - ro_sr);
	delta_sr /= n_sr;
	delta_Ro[1] = 100 * delta_sr / ro_sr;
	service->delta_percent_start[1] = delta_Ro[1];
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AS AS_400 = { 0.0f, }; AS AS_2000 = { 0.0f, };
	//считаем влияние соседнего пласта по палеткам
	if (as_400 > 0 && vzz2layerp == true) {
		AS_400 = calc_Adjacent_Stratum_AF(vzz_2layer_pallete, Phase, range_400, Ro_to_AF, Ro_sr[0], delta_Ro[0],  dfi_bh);
	}

	if (as_2000 > 0 && vzz2layerp == true) {
		AS_2000 = calc_Adjacent_Stratum_AF(vzz_2layer_pallete, Phase, range_2000, Ro_to_AF, Ro_sr[1], delta_Ro[1],  dfi_bh);
	}

	Ro_3c->Ro_p[0] = AS_400.Ro_sonde;
	Ro_3c->R_zp[0] = AS_400.D;
	Ro_3c->Ro_zp[0] = AS_400.Ro_up;
	service->delta_percent_min[0] = AS_400.tf;
	Ro_3c->Ro_p[1] = AS_2000.Ro_sonde;
	Ro_3c->R_zp[1] = AS_2000.D;
	Ro_3c->Ro_zp[1] = AS_2000.Ro_up;
	service->delta_percent_min[1] = AS_2000.tf;
	//cout << "zp.tf " << Zp_400.tf <<" "<< Zp_2000.tf << endl;;
	if (debug == true) {
		for (int i = 0; i < 8; i++) {
			Test << Ro_3c->Ro[i] << " ";
		}
		Test << Ro_3c->ro_sonde[0] << " " << Ro_3c->ro_sonde[1] << " " << Ro_3c->ro_up[0] << " " << Ro_3c->ro_up[1] << " " << Ro_3c->D_[0] << " " << Ro_3c->D_[1] << endl;
	}
	return result;
}
*/

//ok вводим фазу и требуемое УЭС для данной точки и получаем необходимую фазовую поправку
extern "C" __declspec(dllexport) int ph_shift_smt_ph(PHASE *Phase, Ro *Ro_need, PHASE *Phase_shift) {
	int result = 0;
	float dfi_ro_need[2][5];
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_need->Ro[freq][Tx] > 0.00000001 && Phase->Phase[freq][Tx] != 0.0f) {
				dfi_ro_need[freq][Tx] = dFI(param[freq][Tx], Ro_need->Ro[freq][Tx]);
				Phase_shift->Phase[freq][Tx] = Phase->Phase[freq][Tx] - dfi_ro_need[freq][Tx];
			}
			else Phase_shift->Phase[freq][Tx] = 0;
		}
	}
	return 0;
}

//ok вводим УЭС и  требуемое УЭС для данной точки и получаем необходимую фазовую поправку
extern "C" __declspec(dllexport) int ph_shift_smt_ro(Ro *Ro_calk, Ro *Ro_need, PHASE *Phase_shift) {
	int result = 0;
	float dfi_ro_calk[2][5] = { 0, };
	float dfi_ro_need[2][5] = { 0, };
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_need->Ro[freq][Tx] != 0.0f && Ro_calk->Ro[freq][Tx] != 0.0f) {
				dfi_ro_calk[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
				dfi_ro_need[freq][Tx] = dFI(param[freq][Tx], Ro_need->Ro[freq][Tx]);
				Phase_shift->Phase[freq][Tx] = dfi_ro_calk[freq][Tx] - dfi_ro_need[freq][Tx];
			}
			else Phase_shift->Phase[freq][Tx] = 0;
		}
	}
	
	return 0;
}

//по полученному и желаемому УЭС в опорной точке получаем для полученного в любой другой точке УЭС скорректированное (Ro_required)
//УЭС это нужно для посадки на опорную точку - вносим для опорной точки(Ro_calk_ref_point, Ro_need_ref_point) и Ro_calk для искомой 
//точки и получаем Ro_required для искомой точки
//ok Функция работает без sonde_set
extern "C" __declspec(dllexport) int ro_corr_ref_point(void *Metrology, Ro *Ro_calk_ref_point, Ro *Ro_need_ref_point, Ro *Ro_calk,  Ro *Ro_required ) {
	SONDE_PARAM param[2][5] = { 0, };
	
	if (EndsWith((const char*)Metrology, "bin")) {
		ifstream Metro;
		Metro.open((const char*)Metrology, ios::binary);
		if (!Metro.is_open()) {
			if (debug == true) Test << "sonde_set Unable to open Metrology file  " << endl;
			return 1;
		}
		uint32_t signature;
		Metro.read((char*)&signature, sizeof(uint32_t));
		Metro.seekg(0, ios::beg);
		ID id = get_sonde_id(signature);
		//по новому
		if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {
			GP_METROLOGY metrology;
			Metro.read((char*)&metrology, sizeof(GP_METROLOGY));
			if (metrology.D_sonde_mm == 0)
				metrology.D_sonde_mm = 90;//autonomy 1DDS					
			for (int freq = 0; freq < 2; freq++) {
				for (int Tx = 0; Tx < 5; Tx++) {
					param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;
					param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;
					param[freq][Tx].f = float(metrology.F[freq]) * 1000;
					param[freq][Tx].D_sonde_m = float(metrology.D_sonde_mm) / 1000;
					Air[freq][Tx] = metrology.Air_zz[freq][Tx] / mG;
				}
			}
		}
		Metro.close();

	}
	else {
		if (debug == true) Test << "sonde_set Metrology file no .bin ext " << endl;
		return 1;
	}

	int result = 0;
		float dfi_ro_calk_ref_point[2][5] = { 0, };
		float dfi_ro_need_ref_point[2][5] = { 0, };
		float dfi_ro_calk[2][5] = { 0, };
		float dfi_ro_required[2][5] = { 0, };
		float Phase_shift[2][5] = { 0, };
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				if (Ro_need_ref_point->Ro[freq][Tx] != 0) {
					dfi_ro_calk_ref_point[freq][Tx] = dFI(param[freq][Tx], Ro_calk_ref_point->Ro[freq][Tx]);
					dfi_ro_need_ref_point[freq][Tx] = dFI(param[freq][Tx], Ro_need_ref_point->Ro[freq][Tx]);
					dfi_ro_calk[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
					Phase_shift[freq][Tx] = dfi_ro_calk_ref_point[freq][Tx] - dfi_ro_need_ref_point[freq][Tx];
					dfi_ro_required[freq][Tx] = dfi_ro_calk[freq][Tx] - Phase_shift[freq][Tx];
					Ro_required->Ro[freq][Tx] = RO_dFI(param[freq][Tx], dfi_ro_required[freq][Tx]);
					//Ro_required->Ro[][i] = RO_dFI(param[][i], dFI(param[][i], Ro_calk->Ro[][i]) + (dFI(param[][i], Ro_calk_ref_point->Ro[][i]) - dFI(param[][i], Ro_need_ref_point->Ro[][i])));
				}
				else Phase_shift[freq][Tx] = 0;
			}
		}
		
		    
		
		
		return 0;
}

//ok из УЭС получаем симметризованные фазы нужно для операции "КАРАНДАШ"!!!
extern "C" __declspec(dllexport) int ph_smt_ro(Ro *Ro_calk, PHASE *Phase) {
	int result = 0;
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			if (Ro_calk->Ro[freq][Tx] > 0.0f)
				Phase->Phase[freq][Tx] = dFI(param[freq][Tx], Ro_calk->Ro[freq][Tx]);
			else Phase->Phase[freq][Tx] = -32768.00;
		}
	}
	return 0;
}
//ok
extern "C" __declspec(dllexport)  void debug_mode(bool Debug) {
	if(Debug == true)debug = true;
	if(Debug == false)debug = false;
	Test  << " debug = " << debug << endl;
}

//exp//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//УЭС от фазы по золотому сечению просто и без учета скважины и зоны проникновения 
extern "C" __declspec(dllexport) int calculate_Rho_Doll_GR(PHASE *Phase, Ro *Ro_3c) {
	//Golden Ratio
	int result = 0;
	for (int freq = 0; freq < 2; freq++) {
		for (int Tx = 0; Tx < 5; Tx++) {
			Ro_3c->Ro[freq][Tx] = RO_dFI(param[freq][Tx], Phase->Phase[freq][Tx]);
		}
	}

	if (debug == true) {
		for (int freq = 0; freq < 2; freq++) {
			for (int Tx = 0; Tx < 5; Tx++) {
				Test << Ro_3c->Ro[freq][Tx] << " ";
			}
		}
		Test << endl;
	}
	return result;
	
}

extern "C" __declspec(dllexport) int anti_spiral(double *Sgn_in, double *Sgn_out, int length, int win_f, int win_ma) {
	if (win_ma < 1 || win_ma > win_f) win_ma = 1;
	double *Income_buff = new double[win_f];
	double *Output_buff = new double[win_f];
	double *Sgn_ma_buff = new double[win_ma];
	double Sgn_ma_summ = 0;
	double Sgn_out_buff = 0;
	double Out_m;
	int w_ma = 0;
	for (int i = 0; i < win_ma; i++) {
		Sgn_ma_buff[i] = 0;
	}
	for (int i = 0; i < win_f; i++) {
		Income_buff[i] = 0;
		Output_buff[i] = 0;
	}
	for (int n = 0; n < length - win_f; n++) {
		for (int i = 0; i < win_f; i++) {
			Income_buff[i] = Sgn_in[n + i];
		}
		if (n < win_f / 2) {
			Sgn_out[n] = 0;
		}

		if (win_f > 4) {
			if (!harmonics_clear(Income_buff, Output_buff, &Out_m, win_f))
				Sgn_out_buff = Out_m;
			else
				Sgn_out_buff = Sgn_in[n + win_f / 2];
		}
		else
			Sgn_out_buff = Sgn_in[n + win_f / 2];


		//скользящее среднее
		//вычитаем  значение   i ячейки массива окна из суммы всех значений окна скользящего среднего
		Sgn_ma_summ -= Sgn_ma_buff[w_ma];
		//обновляем i ячейку массива окна скользящего среднего
		Sgn_ma_buff[w_ma] = Sgn_out_buff;
		// заносим сырые  G M
		Sgn_ma_summ += Sgn_ma_buff[w_ma];
		// прибавляем обновленное  значениеi ячейки массива к сумме всех значений окна скользящего среднего

		w_ma++;
		if (w_ma == win_ma)w_ma = 0;

		Sgn_out[n + win_f / 2 - win_ma / 2] = Sgn_ma_summ / (double)win_ma;

	}

	for (int n = length - win_f; n < length; n++) {
		Sgn_out[n + win_f / 2 - win_ma / 2] = 0;
	}


	delete Income_buff; delete Output_buff;
	return 0;

}

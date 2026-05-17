#include "stdafx.h"

#include "windows.h"

#include <fstream>

#include <iostream>

#include <iomanip>

#include <cmath>

#include <complex>

#include <string>

#include "function.h"

#include <stdio.h>

#include <time.h>

#include <random> 

#include <bitset>

#include <chrono> 

#include <thread>

#include <mutex>









std::mutex mtx;

using namespace std;





ID get_sonde_id(uint32_t signature) {

	ID tool;

	uint32_t buff;

	buff = signature << 12;

	buff = buff >> 12;

	tool.type_ = buff / 100000;

	tool.N_Tx = (buff % 100000) / 10000;

	tool.mod = (buff % 10000) / 1000;

	tool.number = (buff % 1000);

	tool.type = buff / 1000;

	return tool;

}



//исправить signature -> id.type

int file_in_dir_search(const char *Pallete_dir, string ext, uint32_t search_signature, string *find_file_fullname) {

	extern ofstream Test;

	extern bool debug;



	uint32_t buff_search = search_signature << 12;

	buff_search = buff_search >> 12;

	uint32_t search_signature_for_pallete = buff_search / 1000;





	int result = 0;

	WIN32_FIND_DATAA FindFileData;

	HANDLE hf;



	string local_file_name[32];//локальные имена найденных файлов  с искомым расширением

	string file_fullname[32];//полные имена найденных файлов  с искомым расширением

	string file_fullname_find = "no";//найденное полное имя файла палеток icp

	int number_of_find_files = 0;//количество найденных файлов с искомым расширением

	//ищем файлы с искомым расширением

	hf = FindFirstFileA((Pallete_dir + (std::string)"\\*." + ext).c_str(), &FindFileData);

	if (hf != INVALID_HANDLE_VALUE)

	{

		do {

			local_file_name[number_of_find_files] = FindFileData.cFileName;

			number_of_find_files++;

		} while (FindNextFileA(hf, &FindFileData) != 0);

		FindClose(hf);

	}

	else {

		if (debug == true)Test << "search err_no_files_dir" << endl;

		result = 1;//не нашли дирректории

		return result;

	}



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//выводим количество найденных файлов 

	if (debug == true)Test << "search number_of_files *." + ext + " " << number_of_find_files << endl;



	if (number_of_find_files == 0) {

		if (debug == true)Test << "search err_no_" + ext + "_files_find " << endl;

		result = 2;//не нашли файлов с указанным расширением в данной дирректории

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// формируем полные имена файлов 

	for (int i = 0; i < number_of_find_files; i++) {

		file_fullname[i] = Pallete_dir + (string)"\\" + local_file_name[i];

	}



	ifstream fin;

	// открываем по порядку файлы и ищем подходящую сигнатуру

	for (int i = 0; i < number_of_find_files; i++) {

		fin.open(file_fullname[i], ios::binary);

		if (!fin.is_open()) {

			if (debug == true)Test << "Unable to open " + ext + " file N " << i << endl;

		}

		else {

			uint32_t signature = 0;

			fin.read((char*)&signature, sizeof(signature));

			fin.close();



			uint32_t buff = signature << 12;

			buff = buff >> 12;			

			uint32_t signature_for_pallete = buff / 1000;

			





			if (debug == true) 				

				Test << ext + " file N " << i << +" signature " << signature<<  " search_signature " << search_signature;					

			

			//если сигнатуры совпадают для старых сигнатур

			if (signature == 0x31443453 || signature == 0x31443454) {

				if (search_signature == signature) {

					fin.close();

					//записываем первое найденное подходящее имя

					file_fullname_find = file_fullname[i].c_str();

					if (debug == true)Test << " is same old" << endl;

				}

				else { if (debug == true)Test << " not same old!!! " << endl; }

			}

			

			//если совпадают три цифры для новых сигнатур

			if (search_signature_for_pallete == signature_for_pallete) {

				fin.close();

				//записываем первое найденное подходящее имя

				file_fullname_find = file_fullname[i].c_str();

				if (debug == true)Test << " is same new" << endl;

			}

			else { if (debug == true)Test <<  " not same new!!! " << endl; }

		}

		fin.close();

	}



	if (file_fullname_find == "no") {

		if (debug == true)Test << "no_correct " + ext + " file_find" << endl;//работаем без ЗП!!!!!!!!!!!!!!!!!!!!!!!!

		result = 3;//не нашли файла с подходящей сигнатурой

	}



	//если нашли палетки то заносим из файла в массив

	else {

		if (debug == true)Test << "search file_fullname " + ext + " find " << file_fullname_find << " " <<sizeof (file_fullname_find) << endl;

		memcpy(find_file_fullname, &file_fullname_find, sizeof(file_fullname_find));

		//find_file_fullname = file_fullname_find;

		result = 0;

	}

	if (debug == true)Test << "search  " + ext + " result " << result  << endl;



	return result;



}



// переводит температуру в градусы цельсия

double temp_deg(int adc_value) {

	double temp_deg;

	double v_in = 3300 * adc_value / pow(2, 10) ;

	if (v_in < 1500) temp_deg = (v_in - 500) / 10;

	else if (v_in >= 1500 && v_in < 1752.5) temp_deg = (v_in - 1500) / 10.1 + 100;

	else if (v_in >= 1752.5) temp_deg = (v_in - 1752.5) / 10.6 + 125;

	return temp_deg;

}



uint8_t formula_simmetry_old(float K[4][4], uint8_t condition) {

	uint8_t result;

	//condition = condition >> 4;

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

	///////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////

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

	////////////////////////////////////////////////////////////////////////////////



	else if (condition == 0b00000000) {// выводим несимметризованные значения для всех передатчиков

		K[T1][T1] = 1.00f; K[T1][T2] = 0.00f; K[T1][T3] = 0.00f; K[T1][T4] = 0.00f;

		K[T2][T1] = 0.00f; K[T2][T2] = 1.00f; K[T2][T3] = 0.00f; K[T2][T4] = 0.00f;

		K[T3][T1] = 0.00f; K[T3][T2] = 0.00f; K[T3][T3] = 1.00f; K[T3][T4] = 0.00f;

		K[T4][T1] = 0.00f; K[T4][T2] = 0.00f; K[T4][T3] = 0.00f; K[T4][T4] = 1.00f;

		result = 0b00000000;

	}



	else  {// работает меньше трех передатчиков

		//находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0

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



//принимает condition  как один байт от GP_DATA.uint32_t condition

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

			//находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0

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

			//находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0

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

			//находим рабочие передатчики и для них выводим несимметризованные значения, для нерабочих фаза равна 0

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



int EndsWith(const char *str, const char *suffix)

{

	if (!str || !suffix)

		return 0;

	size_t lenstr = strlen(str);

	size_t lensuffix = strlen(suffix);

	if (lensuffix > lenstr)

		return 0;

	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;

}



// фаза от УЭС для бесконечной среды

float dFI(SONDE_PARAM param, float Ro) {

	int sign = 1;

	const double PI = 3.1415927410125732;

	const double eps0 = 8.85*1e-12;

	const double mu0 = 4 * PI*1e-7;

	const double omega = 2 * PI * static_cast<double>(param.f);

	double sigma = 1.0 / static_cast<double>(Ro);



	//if (sigma > 0)sign = 1;

	//else sign = -1;

	sigma *= sign;

	const std::complex<double> j(0.0, 1.0);

	std::complex<double> ik = j * sqrt(j * omega * mu0 * (sigma - std::complex<double>(0.0, omega * eps0 * 0.0)));

	std::complex<double> ZC1 = exp(ik*(static_cast<double>(param.L1) - static_cast<double>(param.L2))) *  pow((static_cast<double>(param.L2) / static_cast<double>(param.L1)), 3) * ((1.0 - ik * static_cast<double>(param.L1)) / (1.0 - ik * static_cast<double>(param.L2)));

	return static_cast<float>(-arg(ZC1)*sign);

}

//УЭС от фазы по золотому сечению

float  RO_dFI(SONDE_PARAM param, double dfi) {

	int sign = 1;

	//if (dfi > 0)sign = 1;

	//else sign = -1;

	dfi *= sign;

	float Ro0;

	float epsilon_ARG = 0.0000005f; // точность  фазы и амплитуды 

	float  Ro_0 = 0.01f, Ro_max = 7000.0f;  //мин. и мах. значение ус для расчета

	float delta;

	float  ro_0 = Ro_0;

	float  ro_max = Ro_max;

	//  разность фаз 

	do {

		float X1 = ro_0 + 0.382f*(ro_max - ro_0);

		float X2 = ro_max - 0.382f*(ro_max - ro_0);

		float A = static_cast<float>(dfi - dFI(param, X1));

		float B = static_cast<float>(dfi - dFI(param, X2));

		if (fabs(A) > fabs(B)) { ro_0 = X1; }

		else { ro_max = X2; }

		if (A == 0 || B == 0) { delta = 0; }

		else { delta = fabs(A - B); }

	} while (delta > epsilon_ARG);



	Ro0 = (ro_0 + ro_max) / 2;

	ro_0 = Ro_0;

	ro_max = Ro_max;

	return  Ro0*sign;

}

//исправить на другой размер палетки

float Ro_inf_cyl_pallete(INF_CYL_PALLETE *inf_cyl_pallete, int freq, int N_Tx, float signal) {

	if (isnan(signal))return 7200;

	if (signal <= 0)return 7200;

	if (signal > 3)return 0;



	float signal_calc[200];//200 заменить после пересчета палеток

	//бежим по равным ро п и ро зп

	for (int n_Ro_p = 0; n_Ro_p < 200; n_Ro_p++) {

		int n_Ro_zp = n_Ro_p + 60; int n_r_zp = 95;

		signal_calc[n_Ro_p] = (float)(inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].PH[n_r_zp][freq][N_Tx]) / sG;

		if (signal > signal_calc[n_Ro_p]) {

			float signal_a = signal_calc[n_Ro_p - 1] - signal;

			float ctg_a = (inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_p - inf_cyl_pallete->inf_cyl_r[n_Ro_p - 1][n_Ro_zp - 1].Ro_p) /

				(signal_calc[n_Ro_p - 1] - signal_calc[n_Ro_p]);

			float Ro_a = signal_a * ctg_a;

			//std::cout << "Tx " << N_Tx <<" Ro1 " << inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_p <<" Ro2 " << inf_cyl_pallete->inf_cyl_r[n_Ro_p - 1][n_Ro_zp - 1].Ro_p <<" Ro " << Ro_a + inf_cyl_pallete->inf_cyl_r[n_Ro_p - 1][n_Ro_zp - 1].Ro_p << endl;

			return Ro_a + inf_cyl_pallete->inf_cyl_r[n_Ro_p - 1][n_Ro_zp - 1].Ro_p;

			//return inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_p; 

		}

	}

	

	//if (Ro > 7200)Ro = 7200;

	

	

	

}

////////////////////////////////////////////////////////////////////////////////////////////////////////

float DFI_bhole(SONDE_PARAM param, float D_bh_mm, float ro_bh) {

	float dfi = 0;

	float r_bh = static_cast<float>(D_bh_mm / 2000.0);

	float r_sonde = param.D_sonde_m / 2.0f;

	r_sonde = 0.06f;

	float sigma_bh = 1.0f / ro_bh;

	std::cout << "r_sonde " << r_sonde << " r_bh " << r_bh << " sigma_bh " << sigma_bh << " param.f " << param.f << endl;

	float omega = static_cast<float>(2.0 * PI * param.f);

	float L = (param.L1 + param.L2) / 2; float dL = param.L2 - param.L1;

	float P12 = static_cast<float>((omega*mu0* param.L1* param.L1* param.L1) / 4.0);

	float P22 = static_cast<float>((omega*mu0* param.L2* param.L2* param.L2) / 4.0);

	

	//complex <float> sgn_bh = { 0.0,0.0 };

	float sgn_bh = 0.0 ;

	float  dz = 0.01f, dr = 0.001f;

	for (float r = r_sonde; r <= r_bh; r += dr) {

		float QL1 = 0, QL2 = 0;

		for (float z = -5; z < 5; z += dz) {

			float r00 = static_cast<float>(sqrt(pow((z - L), 2) + pow(r, 2)));

			float r01 = static_cast<float>(sqrt(pow((z - dL / 2), 2) + pow(r, 2)));

			float r11 = static_cast<float>(sqrt(pow((z + dL / 2), 2) + pow(r, 2)));

			QL1 += static_cast<float>((P12*(pow(r, 3) * dr * dz)) / pow(r00*r01, 3));

			QL2 += static_cast<float>((P22*(pow(r, 3) * dr * dz)) / pow(r00*r11, 3));

		}

		const std::complex<float> j(0.0f, 1.0f);
		sgn_bh += std::arg((1.0f + j * QL2 * sigma_bh) / (1.0f + j * QL1 * sigma_bh));

	}

	return(sgn_bh);

}



//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Лешина аналитика для нахождения зоны проникновения и истинного ро пласта  (двухслойная цилиндрическая модель с аддитивной скважиной)

// скорее всего перепутаны sigma1 sigma2//



// решение прямой задачи для двухслойной цилиндрической модели - сигнал трехкатушечного зонда 

// для известных Ro_p,  Ro_zp, rzp

float Vzz_inf_cyl(SONDE_PARAM param, float Ro_p, float Ro_zp, float rzp) {

	float Vzz = 0;

	double r0 = (double)rzp;

	double W = 2.0*PI*param.f;

	double dkz1 = 1e-4;

	double dkz2;

	if (rzp <= 0.1 && param.L1 > 0.8) dkz2 = 1.002711275;//2^1/256

	if (rzp <= 0.1 && param.L1 <= 0.8) dkz2 = 1.005429901128;//2^1/128

	if (rzp > 0.1 && rzp <= 0.2) dkz2 = 1.010889286;//2^1/64

	if (rzp > 0.2 && rzp <= 0.4) dkz2 = 1.0218971486;//2^1/32

	if (rzp > 0.4) dkz2 = 1.0442737824;//2^1/16



	double sigma1 = 1.0 / Ro_zp;

	double sigma2 = 1.0 / Ro_p;

	double eps1 = 108.5 * pow(sigma1, 0.35) + 5;

	double eps2 = 108.5 * pow(sigma2, 0.35) + 5;

	std::complex<double> j = { 0.0,1.0 };

	std::complex<double> k1 = (j*W*mu0*(sigma1 - j * W*eps0*eps1));

	std::complex<double> k2 = (j*W*mu0*(sigma2 - j * W*eps0*eps2));

	std::complex<double> inf_L1 = exp(j*sqrt(k1)*(double)param.L1)*(1.0 - j * sqrt(k1)*(double)param.L1) / pow((double)param.L1, 3);

	std::complex<double> inf_L2 = exp(j*sqrt(k1)*(double)param.L2)*(1.0 - j * sqrt(k1)*(double)param.L2) / pow((double)param.L2, 3);



	std::complex<double> integral_L1 = { 0.0,0.0 };

	std::complex<double> integral_L2 = { 0.0,0.0 };



	for (double kz = dkz1; kz < 0.1; kz += dkz1) {

		std::complex<double> x1 = sqrt(pow(kz, 2) - k1);

		std::complex<double> x2 = sqrt(pow(kz, 2) - k2);

		//по новому

		std::complex<double>K1x2r0 = K1(x2*r0);

		std::complex<double>K0x2r0 = K0(x2*r0);

		

		std::complex<double> part_b = pow(x1, 2)*((x1*K1x2r0*K0(x1*r0) - x2 * K0x2r0*K1(x1*r0)) /

			(x1*K1x2r0*I0(x1*r0) + x2 * K0x2r0*I1(x1*r0)));



		integral_L1 += cos(kz*(double)param.L1)*part_b;

		integral_L2 += cos(kz*(double)param.L2)*part_b;

		// iteration1++;

	}

	integral_L1 *= dkz1;

	integral_L2 *= dkz1;



	double dkz = dkz1;

	double kz = 0.1;

	while (kz < 80) {

		kz += dkz;

		dkz *= dkz2;

		std::complex<double> x1 = sqrt(pow(kz, 2) - k1);

		std::complex<double> x2 = sqrt(pow(kz, 2) - k2);

		//по новому

		std::complex<double>K1x2r0 = K1(x2*r0);

		std::complex<double>K0x2r0 = K0(x2*r0);



		std::complex<double> part_b = dkz * pow(x1, 2)*(x1*K1x2r0*K0(x1*r0) - x2 * K0x2r0*K1(x1*r0)) /

			(x1*K1x2r0*I0(x1*r0) + x2 * K0x2r0 *I1(x1*r0));

		

		integral_L1 += cos(kz*(double)param.L1)*part_b;

		integral_L2 += cos(kz*(double)param.L2)*part_b;

		//iteration2++;

	}



	integral_L1 /= PI;

	integral_L2 /= PI;



	integral_L1 += inf_L1;

	integral_L2 += inf_L2;



	Vzz = static_cast<float>(arg(integral_L2 / integral_L1));

	// arg(inf_L2 / inf_L1);

	return Vzz;

}

// для потока

void thread_Vzz_inf_cyl(SONDE_PARAM param, float Ro_p, float Ro_zp, float rzp, float *Vzz) {

	float V_zz;

	V_zz = Vzz_inf_cyl(param, Ro_p, Ro_zp, rzp);

	memcpy(Vzz, &V_zz, sizeof(V_zz));

}

// создание палетки на основе прямой задачи

int create_Vzz_inf_cyl_Pallete(const char *Metrology, const char *Vzz_inf_cyl_pallete_name, bool *start_stop, uint32_t *persent) {

	cout << "metro " << Metrology << endl;

	cout << "ICP " << Vzz_inf_cyl_pallete_name << endl;

	GP_METROLOGY metrology;

	ID id;

	INF_CYL_PALLETE_R *inf_cyl_pallete_r = new INF_CYL_PALLETE_R;

	ifstream fin; 

	ifstream pallete_in;

	ofstream pallete_out;

	INF_CYL_PALLETE_FILE_HEADER header = {};

	//header.N 7536759

	create_bessel_pallete();



	bool new_file = false;

	const uint32_t Ro_p_points = 270; //4023 om

	const uint32_t Ro_zp_points = 288;//4023 om

	float Ro_p_data[Ro_p_points] ; float Ro_zp_data[Ro_zp_points];

	for (float Ro_p = 0.101291f, n_Ro_p = 0.0f; n_Ro_p < Ro_p_points; Ro_p *= 1.04f, n_Ro_p++)

		Ro_p_data[int(n_Ro_p)] = Ro_p;

	for (float Ro_zp = 0.05f, n_Ro_zp = 0.0f; n_Ro_zp < Ro_zp_points; Ro_zp *= 1.04f, n_Ro_zp++)

		Ro_zp_data[int(n_Ro_zp)] = Ro_zp;



	

	if (EndsWith(Metrology, "bin")) {

		fin.open(Metrology, ios::binary);

		if (!fin.is_open()) {

			return 1;//не открылся файл метрологии

		}

		uint32_t signature;

		fin.read((char*)&signature, sizeof(uint32_t));

		fin.seekg(0, ios::beg);

		id = get_sonde_id(signature);

		if (id.type == LWD_4Tx_NEW || id.type == CARTOGRAPH_LWD_4Tx || id.type == AUTONOM_5Tx || id.type == AUTONOM_5Tx_SDR || id.type == LWD_3Tx) {

			metrology;

			fin.read((char*)&metrology, sizeof(GP_METROLOGY));

		}

		fin.close();

	}



	pallete_in.open(Vzz_inf_cyl_pallete_name, ios::binary);//читаем header

	if (!pallete_in.is_open()) {

		// если файла не существует , то его можно создать

		cout << "Vzz_inf_cyl_pallete file not open " << endl;

		pallete_out.open(Vzz_inf_cyl_pallete_name, ios::binary);

		header.tool_type = 0;

		pallete_out.write((char*)&header, sizeof(header));

		pallete_out.close();

		return 1;

	}



	pallete_in.read((char*)&header, sizeof(header));

	pallete_in.close();





	

	//cout << "header.signature " << header.signature << endl;

	if (header.tool_type == 0) {//если header пустой, то заполняем его из файла метрологии

		if (metrology.D_sonde_mm == 0)

			metrology.D_sonde_mm = 90;//autonomy 1DDS					

		for (int freq = 0; freq < 2; freq++) {

			for (int Tx = 0; Tx < 5; Tx++) {

				header.param[freq][Tx].L1 = float(metrology.L1[Tx]) / 1000;

				header.param[freq][Tx].L2 = float(metrology.L2[Tx]) / 1000;

				header.param[freq][Tx].f = float(metrology.F[freq]) * 1000;

				header.param[freq][Tx].D_sonde_m = float(metrology.D_sonde_mm) / 1000;

				std::cout << "header " << "L1 " << header.param[freq][Tx].L1 << " L2 " << header.param[freq][Tx].L2 << " f " << header.param[freq][Tx].f << endl;

			}

		}

		header.tool_type = id.type;

		std::cout << "header.tool_type " << header.tool_type << endl;

		new_file = true;

		header.N = 0;



		pallete_out.open(Vzz_inf_cyl_pallete_name, std::ios::trunc| ios::binary);

		pallete_out.write((char*)&header, sizeof(header));

		cout << " header_size1 " << pallete_out.tellp() << endl;

		pallete_out.close();

	}



	else if (header.tool_type != 0) {//сравниваем 

		if (id.type != header.tool_type) {

			std::cout << "tool_type not same " << endl;

			return 3;//

		}	

	}



	if (new_file == false) {

		pallete_in.open(Vzz_inf_cyl_pallete_name, ios::in, ios::binary);//читаем header

		pallete_in.seekg(-4, ios::end);

		pallete_in.read((char*)&header.N, 4);

		pallete_in.close();

		std::cout << "header.N " << header.N << endl;

	}

	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//для каждого Ro_p набор Ro_zp 

	int N = 2;

	bool work = true;

	for (int n_Ro_p = 0; n_Ro_p < Ro_p_points; n_Ro_p++) {

		inf_cyl_pallete_r->Ro_p = Ro_p_data[n_Ro_p];

		for (int n_Ro_zp = 0; n_Ro_zp < Ro_zp_points; n_Ro_zp++){

			inf_cyl_pallete_r->Ro_zp = Ro_zp_data[n_Ro_zp];

			//std::cout << "Ro_p " << inf_cyl_pallete_r->Ro_p << " Ro_zp " << inf_cyl_pallete_r->Ro_zp << endl;

			///////////////////////////////////////////////////////////////////////////

			if (N <= static_cast<int>(header.N) + 1) {

				N++; 

				std::cout << header.N << " N0 " << N << endl;

			}

			else {

				/////////////////////////////////////

				//auto start = chrono::high_resolution_clock::now();

				/////////////////////////////////////////

				if (*start_stop == true) {

					pallete_out.open(Vzz_inf_cyl_pallete_name, ios::binary | ios::app);

					for (int r_zp = 2, n_r_zp = 0; n_r_zp < 100; r_zp += 2, n_r_zp++) {

						float Vzz[2][5];

						float16_t half_Vzz[2][5];

						std::thread thread1(thread_Vzz_inf_cyl, std::ref(header.param[_400_kGz][0]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_400_kGz][0]);

						std::thread thread2(thread_Vzz_inf_cyl, std::ref(header.param[_400_kGz][1]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_400_kGz][1]);

						std::thread thread3(thread_Vzz_inf_cyl, std::ref(header.param[_400_kGz][2]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_400_kGz][2]);

						std::thread thread4(thread_Vzz_inf_cyl, std::ref(header.param[_400_kGz][3]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_400_kGz][3]);

						std::thread thread5(thread_Vzz_inf_cyl, std::ref(header.param[_400_kGz][4]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_400_kGz][4]);

						std::thread thread6(thread_Vzz_inf_cyl, std::ref(header.param[_2000_kGz][0]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_2000_kGz][0]);

						std::thread thread7(thread_Vzz_inf_cyl, std::ref(header.param[_2000_kGz][1]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_2000_kGz][1]);

						std::thread thread8(thread_Vzz_inf_cyl, std::ref(header.param[_2000_kGz][2]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_2000_kGz][2]);

						std::thread thread9(thread_Vzz_inf_cyl, std::ref(header.param[_2000_kGz][3]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_2000_kGz][3]);

						std::thread thread10(thread_Vzz_inf_cyl, std::ref(header.param[_2000_kGz][4]), inf_cyl_pallete_r->Ro_p, inf_cyl_pallete_r->Ro_zp, (static_cast<float>(r_zp) / 100.0f), &Vzz[_2000_kGz][4]);

						thread1.join(); thread2.join(); thread3.join(); thread4.join(); thread5.join(); thread6.join(); thread7.join(); thread8.join(); thread9.join(); thread10.join();

						for (int freq = 0; freq < 2; freq++) {

							floatToHalf((uint32_t*)(Vzz[freq]), half_Vzz[freq], 5);

							for (int Tx = 0; Tx < 5; Tx++) {

								inf_cyl_pallete_r->PH[n_r_zp][freq][Tx] = half_Vzz[freq][Tx];

							}

						}

							

					}



					pallete_out.write((char*)inf_cyl_pallete_r, sizeof(INF_CYL_PALLETE_R));

					//cout << " file_size " << pallete_out.tellp() << endl;

					pallete_out.close();

					inf_cyl_pallete_r->N = N;

					N++;

					uint32_t buff = N/78;

					std::memcpy(persent, &buff, 4);

					std::cout << header.N << " N1 " << N << " n_Ro_p " << n_Ro_p << " n_Ro_zp " << n_Ro_zp << " % " << *persent << endl;

					//////////////////////////////////////////////////////

					//auto end = chrono::high_resolution_clock::now();

					//chrono::duration<double> duration = end - start;

					//std::cout << "duration test " << duration.count() << endl;

				    ///////////////////////////////////////////////////////////////

				}

				else return 4;

				

			}

		}	

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	pallete_in.open(Vzz_inf_cyl_pallete_name, ios::binary);

	pallete_in.seekg(0, ios::end); //указатель на конец файла

	cout << " file_lenght " << pallete_in.tellg()<< " bytes" << endl;

	pallete_in.close();

	return 0;

}

// вычисляет значение целевой функции по  dfi от зондов 

TF TARGET_FOO_AF(float16_t PH[5], float *Phase, float Ro_p, float Ro_zp, float r_zp, vector <int> range) {

	TF tf = {};

	double Sig_calc[5] = { 0.0f, };

	double Sig_exp[5] = { 0.0f, };

	double target_foo = 0;

	float float_from_half[5] = { 0.0f, };

	halfToFloat(PH, (uint32_t *)float_from_half, 5);

	for (int Tx : range) {

		Sig_exp[Tx] = Phase[Tx];

		Sig_calc[Tx] = float_from_half[Tx];

		target_foo += fabs((Sig_calc[Tx] - Sig_exp[Tx]) / Sig_exp[Tx]);

	}

	if (isnan(target_foo))target_foo = 1000;

	tf.tf = static_cast<float>(target_foo * 100.0);

	tf.Ro_p = Ro_p;

	tf.Ro_zp = Ro_zp;

	tf.R_zp = r_zp ;

	

	//tf.n_r_zp = n_r_zp;

	//tf.n_Ro_p = n_Ro_p;

	//tf.n_Ro_zp = n_Ro_zp;

	//cout<< " R_zp " << tf.R_zp << " Ro_p " << tf.Ro_p << " Ro_zp " << tf.Ro_zp  << " tf.tf " << tf.tf << endl;

	return  tf;

}

//расчет зоны проникновения с участием группы зондов, описанной в range

//inf_cyl_pallete - указатель на файл палеток

//signal указатель на одномерный массив фаз

//Ro_to_AF указатель на одномерный массив - УЭС вычисленные для однородной среды

//Ro_sr указатель на одномерный массив- среднее значение УЭС для однородной среды для группы зондов, участвующих в поиске

//delta_Ro - разброс по УЭС для однородной среды для группы зондов, участвующих в поиске

ZP calc_Penetrition_zone_AF(INF_CYL_PALLETE *inf_cyl_pallete, float *signal, vector <int> range, int freq, float *Ro_to_AF, float Ro_sr, float delta_Ro, float ro_bh, int D_bhole_sm) {

	//auto start1 = chrono::high_resolution_clock::now();

	ZP zp = {};

	STATE_AF state;

	//vector <TF> ENG;

	int n_r_zp_min = 0; int n_Ro_p_min = 0; int n_Ro_zp_min = 0;

	int n_r_zp_max = 100; int n_Ro_p_max = 270; int n_Ro_zp_max = 288;

	// вычисляем ограничения на УЭС пласта для поиска

	float Ro_min = 10000.0f, Ro_max = 0.001f;

	for (int i : range) {

		if (Ro_to_AF[i] >= Ro_max)Ro_max = Ro_to_AF[i];

		if (Ro_to_AF[i] <= Ro_min)Ro_min = Ro_to_AF[i];

	}

	//cout << Ro_min << " " << Ro_max << endl;

	// вычисляем ограничения на УЭС ЗП для поиска

	if (ro_bh < Ro_sr) n_Ro_zp_max = (int)(log(2 * Ro_sr) / log(1.04)) + 77;

	if (n_Ro_zp_max > 287)n_Ro_zp_max = 287;



	n_r_zp_max = static_cast<int>(5 * delta_Ro + 2); 

	n_Ro_p_min = (int)(log(0.3*Ro_min) / log(1.04)) + 59;

	n_Ro_p_max = (int)(log(2 * Ro_max) / log(1.04)) + 59;

	if (n_Ro_p_min < 0)n_Ro_p_min = 0;

	if (n_Ro_p_max > 269)n_Ro_p_max = 269;



	//cout << "Ro_sr " << Ro_sr << " n_Ro_zp_min " << n_Ro_zp_min << " n_Ro_zp_max " << n_Ro_zp_max << endl;



	TF current_Energy = TARGET_FOO_AF(inf_cyl_pallete->inf_cyl_r[0][0].PH[freq][0], signal, 0.0f, 0.0f, 0.0f, range);

	TF candidateEnergy = {};

	

	for (int n_Ro_p = n_Ro_p_min; n_Ro_p < n_Ro_p_max; n_Ro_p += 2) {

		for (int n_Ro_zp = n_Ro_zp_min; n_Ro_zp < n_Ro_zp_max; n_Ro_zp += 2) {

			for (int n_r_zp = n_r_zp_min; n_r_zp < n_r_zp_max; n_r_zp += 2) {

				candidateEnergy = TARGET_FOO_AF(inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].PH[n_r_zp][freq], signal,

					inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_p, inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_zp,  float(n_r_zp)*2, range);

				if (candidateEnergy.tf < current_Energy.tf) {// если кандидат обладает меньшей энергией то оно становится текущим состоянием

					current_Energy = candidateEnergy;

					//ENG.push_back(current_Energy);

					state.n_Ro_p = n_Ro_p; state.n_Ro_zp = n_Ro_zp; state.n_r_zp = n_r_zp;

				}

			}	

		}

	}



	

	int step = 4;

	int n_r_zp_start = state.n_r_zp - step;

	if (n_r_zp_start < 0)n_r_zp_start = 0;

	int n_r_zp_end = state.n_r_zp + step;

	if (n_r_zp_end > 99)n_r_zp_end = 99;



	int n_Ro_p_start = state.n_Ro_p - step;

	if (n_Ro_p_start < 0)n_Ro_p_start = 0;

	int n_Ro_p_end = state.n_Ro_p + step;

	if (n_Ro_p_end > 269)n_Ro_p_end = 270;



	int n_Ro_zp_start = state.n_Ro_zp - step;

	if (n_Ro_zp_start < 0)n_Ro_zp_start = 0;

	int n_Ro_zp_end = state.n_Ro_zp + step;

	if (n_Ro_zp_end > 287)n_Ro_zp_end = 288;





	for (int n_Ro_p = n_Ro_p_start; n_Ro_p < n_Ro_p_end; n_Ro_p ++) {

		for (int n_Ro_zp = n_Ro_zp_start; n_Ro_zp < n_Ro_zp_end; n_Ro_zp ++) {

			for (int n_r_zp = n_r_zp_start; n_r_zp < n_r_zp_end; n_r_zp ++) {

				candidateEnergy = TARGET_FOO_AF(inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].PH[n_r_zp][freq], signal,

					inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_p, inf_cyl_pallete->inf_cyl_r[n_Ro_p][n_Ro_zp].Ro_zp, float(n_r_zp) *2, range);

				if (candidateEnergy.tf < current_Energy.tf) {// если кандидат обладает меньшей энергией то оно становится текущим состоянием

					current_Energy = candidateEnergy;

					//ENG.push_back(current_Energy);

					state.n_Ro_p = n_Ro_p; state.n_Ro_zp = n_Ro_zp; state.n_r_zp = n_r_zp;

				}

			}

		}

	}

	

	zp.R_zp = current_Energy.R_zp;

	zp.Ro_p = current_Energy.Ro_p;

	zp.Ro_zp = current_Energy.Ro_zp;

	zp.tf = current_Energy.tf;



///////////////////////////////////////////////////////////////////////////////////////

	//cout << "zp.tf " << zp.tf << endl;



	//auto end1 = chrono::high_resolution_clock::now();

	//chrono::duration<double> duration = end1 - start1;

	//cout << "duration dll " << duration.count() << endl;



	/*

	cout << "ENG.size() " << ENG.size() << endl;;

	for (size_t i = 0; i < ENG.size(); i++) {

		cout << i << " n_r_zp " << ENG[i].n_r_zp << " n_Ro_p " << ENG[i].n_Ro_p << " n_Ro_zp " << ENG[i].n_Ro_zp << endl;

		cout << " Ro_p " << ENG[i].Ro_p << " Ro_zp " << ENG[i].Ro_zp << " R_zp " << ENG[i].R_zp << " tf.tf " << ENG[i].tf << endl;



	}

	cout << endl;

	*/

	return zp;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// решение прямой задачи для двухслойной  модели с плоской границей для коррекциии УЭС вблизи соседнего пласта 

//для горизонтальной скважины - сигнал трехкатушечного зонда  для известных Ro_sonde, Ro_up, и расстоянии до границы

float Vzz_2layer(SONDE_PARAM param, int N_sonde, float Ro_sonde, float Ro_up, float D_) {

	int n = 0;

	double D = (double)D_;

	double W = 2.0*PI*param.f;

	double dl = 1e-4;

	//double dl2 = pow(2, 0.0625);//2^1/16

	double dl2 = pow(2, 1.0 / 64);//2^1/64

	//double L = 2.35;

	double sigma_sonde = 1.0 / Ro_sonde;

	double sigma_up = 1.0 / Ro_up;

	double eps_sonde = 108.5 * pow(sigma_sonde, 0.35) + 5;

	double eps_up = 108.5 * pow(sigma_up, 0.35) + 5;

	std::complex<double> j = { 0.0,1.0 };

	std::complex<double> k1 = (j*W*mu0*sigma_sonde);

	std::complex<double> k2 = (j*W*mu0*(sigma_up));

	//std::complex<double> k1 = (j*W*mu0*(sigma_sonde - j * W*eps0*eps_sonde));

	//std::complex<double> k2 = (j*W*mu0*(sigma_up - j * W*eps0*eps_up));

	std::complex<double> K1 = j * W * mu0 / (4 * PI);

	std::complex<double> chlen1_L1 = -2.0  * exp(j*sqrt(k1)*(double)param.L1)*(1.0 - j * sqrt(k1)*(double)param.L1) / (pow((double)param.L1, 3));

	std::complex<double> chlen1_L2 = -2.0  * exp(j*sqrt(k1)*(double)param.L2)*(1.0 - j * sqrt(k1)*(double)param.L2) / (pow((double)param.L2, 3));

	std::complex<double> integral_L1 = { 0.0,0.0 };

	std::complex<double> integral_L2 = { 0.0,0.0 };



	for (double l = dl; l < 0.1; l += dl) {

		std::complex<double> p1 = sqrt(l*l - k1);

		std::complex<double> p2 = sqrt(l*l - k2);

		std::complex<double> K_H_12 = (p2 - p1) / (p2 + p1);

		std::complex<double> K_E_12 = (sigma_sonde*p2 - sigma_up * p1) / (sigma_sonde*p2 + sigma_up * p1);



		//pallete.Vzz_3coil_bessel_J1lL1[sonde][n]



		integral_L1 += (pallete.Vzz_3coil_bessel_J1lL1[N_sonde][n] * K_E_12*k1 / (p1*(double)param.L1)

			+ (pallete.Vzz_3coil_bessel_J0lL1[N_sonde][n] - pallete.Vzz_3coil_bessel_J2lL1[N_sonde][n]) *K_H_12*l*p1*0.5) * exp(-2.0*p1*D);

		integral_L2 += (pallete.Vzz_3coil_bessel_J1lL2[2][n] * K_E_12*k1 / (p1*(double)param.L2)

			+ (pallete.Vzz_3coil_bessel_J0lL2[N_sonde][n] - pallete.Vzz_3coil_bessel_J2lL2[N_sonde][n]) *K_H_12*l*p1*0.5) * exp(-2.0*p1*D);

		n++;

	}

	integral_L1 *= dl; integral_L2 *= dl;



	double l = 0.1;

	while (l < 160) {

		std::complex<double> p1 = sqrt(l*l - k1);

		std::complex<double> p2 = sqrt(l*l - k2);

		std::complex<double> K_H_12 = (p2 - p1) / (p2 + p1);

		std::complex<double> K_E_12 = (sigma_sonde*p2 - sigma_up * p1) / (sigma_sonde*p2 + sigma_up * p1);



		l += dl;

		dl *= dl2;

		integral_L1 += (pallete.Vzz_3coil_bessel_J1lL1[N_sonde][n] * K_E_12*k1 / (p1*(double)param.L1)

			+ (pallete.Vzz_3coil_bessel_J0lL1[N_sonde][n] - pallete.Vzz_3coil_bessel_J2lL1[N_sonde][n]) *K_H_12*l*p1*0.5) * exp(-2.0*p1*D)*dl;

		integral_L2 += (pallete.Vzz_3coil_bessel_J1lL2[N_sonde][n] * K_E_12*k1 / (p1*(double)param.L2)

			+ (pallete.Vzz_3coil_bessel_J0lL2[N_sonde][n] - pallete.Vzz_3coil_bessel_J2lL2[N_sonde][n]) *K_H_12*l*p1*0.5) * exp(-2.0*p1*D)*dl;

		n++;

	}

	//cout << "n " << n << endl;



	return (float)(arg((integral_L2 + chlen1_L2) / (integral_L1 + chlen1_L1)));



}

// для потока

void thread_Vzz_2layer(SONDE_PARAM param, int N_sonde, float Ro_sonde, float Ro_up, float D, float *Vzz) {

	float V_zz;

	V_zz = Vzz_2layer(param, N_sonde, Ro_sonde, Ro_up, D);

	memcpy(Vzz, &V_zz, sizeof(V_zz));

}

// создание палетки на основе прямой задачи

int create_Vzz_2layer_Pallete(const char *Metrology, const char *Vzz_2layer_pallete_name, bool *start_stop, uint32_t *persent) {

	bool new_file = false;

	const uint32_t Ro_sonde_points = 225; //4023 om

	const uint32_t Ro_up_points = 225;//4023 om

	float Ro_sonde_data[Ro_sonde_points]; float Ro_up_data[Ro_up_points];

	for (float Ro_sonde = 0.3f, n_Ro_sonde = 0.0f; n_Ro_sonde < Ro_sonde_points; Ro_sonde *= 1.04f, n_Ro_sonde++)

		Ro_sonde_data[int(n_Ro_sonde)] = Ro_sonde;

	for (float Ro_up = 0.3f, n_Ro_up = 0.0f; n_Ro_up < Ro_up_points; Ro_up *= 1.04f, n_Ro_up++)

		Ro_up_data[int(n_Ro_up)] = Ro_up;



	GP_METROLOGY metrology_struct;

	std::ifstream Metro;

	Metro.open(Metrology, std::ios::binary);

	if (!Metro.is_open()) {

		return 1;

	}

	Metro.read((char*)&metrology_struct, sizeof(metrology_struct));

	Metro.close();



	/*

	if (metrology_struct.signature == 0x33444453 ||

		metrology_struct.signature == 0x324C5744) {



	}

	else {

		std::cout << "create_dfi_ro_pallete Metrology signature not correct " << metrology_struct.signature << endl;

		return 2;

	}

	*/



	VZZ_2LAYER_PALLETE_UNIT *vzz_2layer_pallete_unit = new VZZ_2LAYER_PALLETE_UNIT;

	std::ifstream pallete_in;

	std::ofstream pallete_out;

	VZZ_2LAYER_PALLETE_FILE_HEADER header = {};



	pallete_in.open(Vzz_2layer_pallete_name, std::ios::binary);//читаем header

	if (!pallete_in.is_open()) {

		// если файла не существует , то его можно создать

		std::cout << "Vzz_2layer_pallete file not open " << std::endl;

		pallete_out.open(Vzz_2layer_pallete_name, std::ios::binary);

		pallete_out.close();

		return 1;

	}

	pallete_in.read((char*)&header, sizeof(header));

	pallete_in.close();





	//cout << "header.signature " << header.signature << endl;

	if (header.signature == 0) {//если header пустой, то заполняем его из файла метрологии

		for (int i : { T1_400, T2_400, T3_400, T4_400, T1_2000, T2_2000, T3_2000, T4_2000 }) {

			header.param[i].L1 = static_cast<float>(metrology_struct.L1[i] / 1000.0);

			header.param[i].L2 = static_cast<float>(metrology_struct.L2[i] / 1000.0);

			header.param[i].f = static_cast<float>(1000.0 * metrology_struct.F[i]);

			header.param[i].D_sonde_m = static_cast<float>(metrology_struct.D_sonde_mm / 1000.0);

			std::cout << "header " << "L1 " << header.param[i].L1 << " L2 " << header.param[i].L2 << " f " << header.param[i].f << std::endl;

		}

		header.signature = metrology_struct.signature;

		header.serial = metrology_struct.serial;

		new_file = true;

		header.N = 0;



		create_Vzz_3coil_bessel_pallete(header.param, 8);





		pallete_out.open(Vzz_2layer_pallete_name, std::ios::binary);

		pallete_out.write((char*)&header, sizeof(header));

		std::cout << " header_size1 " << pallete_out.tellp() << std::endl;

		pallete_out.close();

	}



	else if (header.signature != 0) {//сравниваем 

		if (metrology_struct.signature != header.signature) {

			std::cout << "segnatures not same " << std::endl;

			return 3;//

		}



	}



	if (new_file == false) {

		pallete_in.open(Vzz_2layer_pallete_name, std::ios::in, std::ios::binary);//читаем header

		pallete_in.seekg(-4, std::ios::end);

		pallete_in.read((char*)&header.N, 4);

		pallete_in.close();

		std::cout << "header.N " << header.N << std::endl;

	}



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//для каждого Ro_p набор Ro_zp 

	int N = 2;

	bool work = true;

	for (int n_Ro_sonde = 0; n_Ro_sonde < Ro_sonde_points; n_Ro_sonde++) {

		vzz_2layer_pallete_unit->Ro_sonde = Ro_sonde_data[n_Ro_sonde];

		for (int n_Ro_up = 0; n_Ro_up < Ro_up_points; n_Ro_up++) {

			vzz_2layer_pallete_unit->Ro_up = Ro_up_data[n_Ro_up];

			//std::cout << "Ro_p " << inf_cyl_pallete_r->Ro_p << " Ro_zp " << inf_cyl_pallete_r->Ro_zp << endl;

			///////////////////////////////////////////////////////////////////////////

			if (N <= static_cast<int>(header.N) + 1) {

				N++;

				std::cout << header.N << " N0 " << N << std::endl;

			}

			else {

				/////////////////////////////////////

				//auto start = chrono::high_resolution_clock::now();

				/////////////////////////////////////////

				if (*start_stop == true) {

					pallete_out.open(Vzz_2layer_pallete_name, std::ios::binary | std::ios::app);

					for (int D = 5, n_D = 0; n_D < 100; D += 5, n_D++) {

						float Vzz[8];						

						std::thread thread1(thread_Vzz_2layer, std::ref(header.param[0]), 0, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[0]);

						std::thread thread2(thread_Vzz_2layer, std::ref(header.param[1]), 1, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[1]);

						std::thread thread3(thread_Vzz_2layer, std::ref(header.param[2]), 2, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[2]);

						std::thread thread4(thread_Vzz_2layer, std::ref(header.param[3]), 3, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[3]);

						std::thread thread5(thread_Vzz_2layer, std::ref(header.param[4]), 4, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[4]);

						std::thread thread6(thread_Vzz_2layer, std::ref(header.param[5]), 5, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[5]);

						std::thread thread7(thread_Vzz_2layer, std::ref(header.param[6]), 6, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[6]);

						std::thread thread8(thread_Vzz_2layer, std::ref(header.param[7]), 7, vzz_2layer_pallete_unit->Ro_sonde, vzz_2layer_pallete_unit->Ro_up, (static_cast<float>(D) / 100.0f), &Vzz[7]);

						thread1.join(); thread2.join(); thread3.join(); thread4.join(); thread5.join(); thread6.join(); thread7.join(); thread8.join();						

						for (int N_sonde = 0; N_sonde < 8; N_sonde++)

							vzz_2layer_pallete_unit->PH[n_D][N_sonde] = Vzz[N_sonde];

					}

					pallete_out.write((char*)vzz_2layer_pallete_unit, sizeof(VZZ_2LAYER_PALLETE_UNIT));

					//cout << " file_size " << pallete_out.tellp() << endl;

					pallete_out.close();

					vzz_2layer_pallete_unit->N = N;

					N++;

					uint32_t buff = N / 78;

					std::memcpy(persent, &buff, 4);

					std::cout << header.N << " N1 " << N << " n_Ro_sonde " << n_Ro_sonde << " n_Ro_up " << n_Ro_up << " % " << *persent << std::endl;

					//////////////////////////////////////////////////////

					//auto end = chrono::high_resolution_clock::now();

					//chrono::duration<double> duration = end - start;

					//std::cout << "duration test " << duration.count() << endl;

					///////////////////////////////////////////////////////////////

				}

				else return 4;



			}

		}

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	pallete_in.open(Vzz_2layer_pallete_name, std::ios::binary);

	pallete_in.seekg(0, std::ios::end); //указатель на конец файла

	std::cout << " file_lenght " << pallete_in.tellg() << " bytes" << std::endl;

	pallete_in.close();

	return 0;

}

// вычисляет значение целевой функции по 4 dfi от зондов 

TF TARGET_FOO_VZZ_2LAYER(float PH[8], float *signal, float Ro_sonde, float Ro_up, float D, float dfi_bh[5], vector <int> range) {

	TF tf = {};

	double Sig_calc[8] = { 0.0f, };

	double Sig_exp[8] = { 0.0f, };

	double target_foo = 0;

	int n_sondes = 0;

	for (int i : range) {

		n_sondes++;

		Sig_exp[i] = signal[i] - dfi_bh[i];

		Sig_calc[i] = PH[i];

		target_foo += fabs((Sig_calc[i] - Sig_exp[i]) / Sig_exp[i]);

	}

	if (isnan(target_foo))target_foo = 1000;

	tf.tf = (float)((target_foo*100.0)/ n_sondes);

	tf.Ro_sonde = Ro_sonde;

	tf.Ro_up = Ro_up;

	tf.D = D;

	//tf.n_r_zp = n_r_zp;

	//tf.n_Ro_p = n_Ro_p;

	//tf.n_Ro_zp = n_Ro_zp;

	//cout<< " R_zp " << tf.R_zp << " Ro_p " << tf.Ro_p << " Ro_zp " << tf.Ro_zp  << " tf.tf " << tf.tf << endl;

	return  tf;

}

//

AS calc_Adjacent_Stratum_AF(VZZ_2LAYER_PALLETE *vzz_2layer_pallete, float *signal, vector <int> range, float *Ro_to_AF, float Ro_sr, float delta_Ro,  float dfi_bh[8]) {

	//auto start1 = chrono::high_resolution_clock::now();

	AS as = {};

	STATE_AF state;

	//vector <TF> ENG;

	int n_D_min = 0; int n_Ro_sonde_min = 0; int n_Ro_up_min = 0;

	int n_D_max = 100; int n_Ro_sonde_max = 225; int n_Ro_up_max = 225; 

	/*

	float Ro_min = 10000.0f, Ro_max = 0.001f;

	for (int i : range) {

		if (Ro_to_AF[i] >= Ro_max)Ro_max = Ro_to_AF[i];

		if (Ro_to_AF[i] <= Ro_min)Ro_min = Ro_to_AF[i];	//cout << Ro_min << " " << Ro_max << endl;

	if (ro_bh < Ro_sr) n_Ro_zp_max = (int)(log(2 * Ro_sr) / log(1.04)) + 77;

	if (n_Ro_zp_max > 287)n_Ro_zp_max = 287;



	n_r_zp_max = 5 * delta_Ro + 2;

	n_Ro_p_min = (int)(log(0.3*Ro_min) / log(1.04)) + 59;

	n_Ro_p_max = (int)(log(2 * Ro_max) / log(1.04)) + 59;

	if (n_Ro_p_min < 0)n_Ro_p_min = 0;

	if (n_Ro_p_max > 269)n_Ro_p_max = 269;

	*/

	//cout << "Ro_sr " << Ro_sr << " n_Ro_zp_min " << n_Ro_zp_min << " n_Ro_zp_max " << n_Ro_zp_max << endl;





	 n_Ro_up_min = (int)(log( Ro_sr/0.3) / log(1.04))  ;

	 n_Ro_up_max = 225;



	TF current_Energy = TARGET_FOO_VZZ_2LAYER(vzz_2layer_pallete->vzz_2layer_unit[0][0].PH[0], signal, 0.0f, 0.0f, 0.0f, dfi_bh, range);

	TF candidateEnergy = {};



	for (int n_Ro_sonde = n_Ro_sonde_min; n_Ro_sonde < n_Ro_sonde_max; n_Ro_sonde += 1) {

		for (int n_Ro_up = n_Ro_up_min; n_Ro_up < n_Ro_up_max; n_Ro_up += 1) {

			for (int n_D = n_D_min; n_D < n_D_max; n_D += 1) {

				candidateEnergy = TARGET_FOO_VZZ_2LAYER(vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].PH[n_D], signal,                                         //?

					vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].Ro_sonde, vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].Ro_up, float(n_D) * 5, dfi_bh, range);

				if (candidateEnergy.tf < current_Energy.tf) {// если кандидат обладает меньшей энергией то оно становится текущим состоянием

					current_Energy = candidateEnergy;

					//ENG.push_back(current_Energy);

					state.n_Ro_sonde = n_Ro_sonde; state.n_Ro_up = n_Ro_up; state.n_D = n_D;

				}

			}



		}

	}



	

	int step = 4;

	int n_D_start = state.n_D - step;

	if (n_D_start < 0)n_D_start = 0;

	int n_D_end = state.n_D + step;

	if (n_D_end > 99)n_D_end = 99;



	int n_Ro_sonde_start = state.n_Ro_sonde - step;

	if (n_Ro_sonde_start < 0)n_Ro_sonde_start = 0;

	int n_Ro_sonde_end = state.n_Ro_sonde + step;

	if (n_Ro_sonde_end > 225)n_Ro_sonde_end = 225;



	int n_Ro_up_start = state.n_Ro_up - step;

	if (n_Ro_up_start < 0)n_Ro_up_start = 0;

	int n_Ro_up_end = state.n_Ro_up + step;

	if (n_Ro_up_end > 225)n_Ro_up_end = 225;





	for (int n_Ro_sonde = n_Ro_sonde_start; n_Ro_sonde < n_Ro_sonde_end; n_Ro_sonde++) {

		for (int n_Ro_up = n_Ro_up_start; n_Ro_up < n_Ro_up_end; n_Ro_up++) {

			for (int n_D = n_D_start; n_D < n_D_end; n_D++) {

				candidateEnergy = TARGET_FOO_VZZ_2LAYER(vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].PH[n_D], signal,

					vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].Ro_sonde, vzz_2layer_pallete->vzz_2layer_unit[n_Ro_sonde][n_Ro_up].Ro_up, float(n_D) * 5, dfi_bh, range);

				if (candidateEnergy.tf < current_Energy.tf) {// если кандидат обладает меньшей энергией то оно становится текущим состоянием

					current_Energy = candidateEnergy;

					//ENG.push_back(current_Energy);

					state.n_Ro_sonde = n_Ro_sonde; state.n_Ro_up = n_Ro_up; state.n_D = n_D;

				}

			}



		}

	}



	

	as.D = current_Energy.D;

	as.Ro_sonde = current_Energy.Ro_sonde;

	as.Ro_up = current_Energy.Ro_up;

	as.tf = current_Energy.tf;

	///////////////////////////////////////////////////////////////////////////////////////

		//cout << "zp.tf " << zp.tf << endl;



		//auto end1 = chrono::high_resolution_clock::now();

		//chrono::duration<double> duration = end1 - start1;

		//cout << "duration dll " << duration.count() << endl;



		/*

		cout << "ENG.size() " << ENG.size() << endl;;

		for (size_t i = 0; i < ENG.size(); i++) {

			cout << i << " n_r_zp " << ENG[i].n_r_zp << " n_Ro_p " << ENG[i].n_Ro_p << " n_Ro_zp " << ENG[i].n_Ro_zp << endl;

			cout << " Ro_p " << ENG[i].Ro_p << " Ro_zp " << ENG[i].Ro_zp << " R_zp " << ENG[i].R_zp << " tf.tf " << ENG[i].tf << endl;



		}

		cout << endl;

		*/

	return as;

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//anti spiral

void DFT(double *SGN, complex <double> *Harm, int win) {

	double win_2 = win / 2.0;

	for (int i = 0; i < win_2; i++)

		Harm[i] = 0;

	for (int k = 0; k < win_2; k++) {

		for (int n = 0; n < win; n++) {

			Harm[k] += SGN[n] * exp((std::complex<double>(0.0, -1.0) * PI * double(k*(n))) / win_2);

		}

		Harm[k] /= win_2;

	}

}



int SLAU(double matrica_a[5][5], int n, double massiv_b[5], double x[5]) {

	n = 5;

	int i, j, k, r;

	double c = 0, M = 0, max = 0, s = 0, a[5][5] = { 0.0f, }, b[5] = { 0.0f, };

	for (i = 0; i < n; i++) { x[i] = 0; }

	for (i = 0; i < n; i++) {

		for (j = 0; j < n; j++) {

			a[i][j] = matrica_a[i][j];

		}

	}



	for (i = 0; i < n; i++) {

		b[i] = massiv_b[i];

	}



	for (k = 0; k < n; k++)

	{

		max = fabs(a[k][k]);

		r = k;

		for (i = k + 1; i < n; i++) {

			if (fabs(a[i][k]) > max) {

				max = fabs(a[i][k]);

				r = i;

			}

		}



		for (j = 0; j < n; j++) {

			c = a[k][j]; a[k][j] = a[r][j]; a[r][j] = c;

		}



		c = b[k]; b[k] = b[r]; b[r] = c;



		for (i = k + 1; i < n; i++) {

			for (M = a[i][k] / a[k][k], j = k; j < n; j++) {

				a[i][j] -= M * a[k][j];

			}

			b[i] -= M * b[k];

		}

	}



	if (a[n - 1][n - 1] == 0)



		if (b[n - 1] == 0) return -1;



		else return -2;



	else {

		for (i = n - 1; i >= 0; i--) {

			for (s = 0, j = i + 1; j < n; j++) {

				s += a[i][j] * x[j];

			}

			x[i] = (b[i] - s) / a[i][i];

		}

		return 0;

	}





}



int harmonics_clear(double *Sgn, double *Sgn_out, double *Sgn_out_m, int win) {

	complex <double> *HARM = new complex <double>[win];

	//complex <double> HARM[16];

	double min = 0;

	double max = 0;

	int n_max = 0;

	int n_remove = 0;

	bool stop = false;

	double win_2 = win / 2.0;



	//////////////////////////////////////////////////////////////////////////////////////////////

	DFT(Sgn, HARM, win);



	for (int i = 0; i < win_2; i++) {

		if (abs(HARM[i + 1]) <= abs(HARM[i + 2]) && abs(HARM[i + 1]) <= abs(HARM[i]) && stop == false) {

			n_remove = i + 1; stop = true;

		}

	}

	for (int i = n_remove; i < win_2; i++) {

		if (abs(HARM[i]) > max) {

			max = abs(HARM[i]);

			n_max = i;

		}

	}



	double T = win / (n_max - real((HARM[n_max + 1] - HARM[n_max - 1]) / (2.0 * HARM[n_max] - HARM[n_max - 1] - HARM[n_max + 1])));

	if (T <= 0) return 1;

	////////////////////////////////////////////////////////////////////////////////////////////

	double B[5][5] = { 0.0f, }, D[5] = { 0.0f, }; double X[5] = { 0.0f, };

	double Scosfi = 0, Ssinfi = 0, Ssinficosfi = 0, SsinKfi = 0, ScosKfi = 0;

	double Scos2ficosfi = 0, Ssin2fisinfi = 0, Ssin2ficosfi = 0, Scos2fisinfi = 0;

	double Ssin2ficos2fi = 0, Ssin2fi = 0, Scos2fi = 0, SsinK2fi = 0, ScosK2fi = 0;

	double V_ = 0, V_cosfi = 0, V_sinfi = 0, V_cos2fi = 0, V_sin2fi = 0;

	double Angle = 0;



	for (int i = 0; i < win; i++) {

		Angle = double(i) * 2.0 * PI / T;

		B[0][1] += cos(Angle);

		B[0][2] += sin(Angle);

		B[1][2] += sin(Angle)*cos(Angle);

		B[1][1] += pow(cos(Angle), 2);

		B[2][2] += pow(sin(Angle), 2);



		B[1][3] += cos(2 * Angle)*cos(Angle);

		B[2][4] += sin(2 * Angle)*sin(Angle);

		B[1][4] += sin(2 * Angle)*cos(Angle);

		B[2][3] += cos(2 * Angle)*sin(Angle);

		B[3][4] += sin(2 * Angle)*cos(2 * Angle);

		B[0][4] += sin(2 * Angle);

		B[0][3] += cos(2 * Angle);

		B[4][4] += pow(sin(2 * Angle), 2);

		B[3][3] += pow(cos(2 * Angle), 2);



		D[0] += Sgn[i];

		D[1] += Sgn[i] * cos(Angle);

		D[2] += Sgn[i] * sin(Angle);

		D[3] += Sgn[i] * cos(2 * Angle);

		D[4] += Sgn[i] * sin(2 * Angle);



	}

	B[0][0] = win;

	B[1][0] = B[0][1]; B[2][0] = B[0][2]; B[3][0] = B[0][3]; B[4][0] = B[0][4];

	B[2][1] = B[1][2]; B[3][1] = B[1][3]; B[4][1] = B[1][4];

	B[3][2] = B[2][3]; B[4][2] = B[2][4]; B[4][3] = B[3][4];

	////////////////////////////////////////////////////////////////////////////////////////	

	if (SLAU(B, 5, D, X) != 0) {

		return 1;

	}



	if (isnan(X[0]) || isnan(X[1]) || isnan(X[2]) || isnan(X[3]) || isnan(X[4]))return 1;



	//cout << "P1=" << X[0] << " C1=" << X[1] << " S1=" << X[2] << " C2=" << X[3] << " S2=" << X[4] << endl;

	for (int i = 0; i < win; i++) {

		double angle = double(i) * 2.0 * PI / T;

		//Sgn_out[i] = X[0] + X[1] * cos(angle) + X[2] * sin(angle) + X[3] * cos(2 * angle) + X[4] * sin(2 * angle);

		Sgn_out[i] = X[1] * cos(angle) + X[2] * sin(angle) + X[3] * cos(2 * angle) + X[4] * sin(2 * angle);

	}



	*Sgn_out_m = Sgn[(int)(win_2)] - Sgn_out[(int)(win_2)];

	delete HARM;

	return 0;

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////

//направленные свойства




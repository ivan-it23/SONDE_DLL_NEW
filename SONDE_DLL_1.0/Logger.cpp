#include "stdafx.h"
#include "Logger.h"
#include "Constants.h"

#include <chrono>
#include <ctime>

#pragma warning(disable : 4996)

using namespace std;
using namespace std::chrono;

ofstream Test;
bool debug = false;
const char* Test_Name = config::kLogFileName;

namespace logger {

static time_t start_time = 0;
static time_t stop_time = 0;

void init() {
	// чистим файл лога при загрузке библиотеки
	Test.open(Test_Name, ios::out | ios::trunc);
	start_time = system_clock::to_time_t(system_clock::now());
	Test << "lib is open " << ctime(&start_time) << endl;
}

void shutdown() {
	stop_time = system_clock::to_time_t(system_clock::now());
	Test << "lib is close " << ctime(&stop_time) << endl;
	Test.close();
}

} // namespace logger

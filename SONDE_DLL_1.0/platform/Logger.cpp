#include "stdafx.h"
#include "Logger.h"
#include "Constants.h"

using namespace std;

SafeLog Test;
atomic<bool> debug(false);
const char* Test_Name = config::kLogFileName;

SafeLog::SafeLog() noexcept : stream_(nullptr) {}

// Не закрываем файл из статического деструктора DLL под loader lock.
// Штатное закрытие выполняется debug_mode(false), а при выгрузке дескриптор
// в любом случае освобождает операционная система.
SafeLog::~SafeLog() noexcept = default;

SafeLog& SafeLog::operator<<(ostream& (*manipulator)(ostream&)) {
	lock_guard<mutex> lock(mutex_);
	if (stream_ != nullptr && stream_->is_open()) manipulator(*stream_);
	return *this;
}

void SafeLog::open(const char* path) {
	lock_guard<mutex> lock(mutex_);
	if (stream_ == nullptr) stream_ = new ofstream();
	if (stream_->is_open()) stream_->close();
	stream_->open(path, ios::out | ios::trunc);
}

void SafeLog::close() noexcept {
	lock_guard<mutex> lock(mutex_);
	if (stream_ != nullptr && stream_->is_open()) {
		stream_->flush();
		stream_->close();
	}
}

namespace logger {

void set_enabled(bool enabled) {
	if (enabled) {
		Test.open(Test_Name);
		debug.store(true, memory_order_release);
		Test << "debug logging enabled" << endl;
	} else {
		debug.store(false, memory_order_release);
		Test.close();
	}
}

} // namespace logger

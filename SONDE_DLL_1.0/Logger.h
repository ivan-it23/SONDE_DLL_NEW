#pragma once
// Logger.h
// Единая точка управления диагностическим логом DLL (файл Test.txt) и флагом
// отладки. Глобальные имена Test и debug сохранены для совместимости с кодом,
// который обращается к ним через extern.

#include <atomic>
#include <fstream>
#include <mutex>

// Совместим с существующими выражениями `Test << ...`, но сериализует запись.
// Файл открывается только явным вызовом debug_mode(true), вне DllMain.
class SafeLog {
public:
	SafeLog() noexcept;
	~SafeLog() noexcept;

	template <typename T>
	SafeLog& operator<<(const T& value) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (stream_ != nullptr && stream_->is_open()) (*stream_) << value;
		return *this;
	}

	SafeLog& operator<<(std::ostream& (*manipulator)(std::ostream&));
	void open(const char* path);
	void close() noexcept;

private:
	std::mutex mutex_;
	std::ofstream* stream_;
};

extern SafeLog Test;                  // поток диагностического лога
extern std::atomic<bool> debug;        // признак включённой отладки (debug_mode)
extern const char* Test_Name;  // имя файла лога

namespace logger {

void set_enabled(bool enabled);

} // namespace logger

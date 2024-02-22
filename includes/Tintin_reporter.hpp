#pragma once

#include "../includes/matt_daemon.hpp"
#include <memory>

enum LogLevel { INFO, LOG, ERROR };

class Tintin_reporter {
public:
	Tintin_reporter(const std::string& filename) {
		_file.open(filename, std::ios::out | std::ios::app);
		if (!_file.is_open()) fileError("open", filename.c_str());
	}

	~Tintin_reporter() {
		if (_file.is_open()) _file.close();
	}

	void vlog(LogLevel level, const char* format, va_list args) {
		va_list args_copy;
		va_copy(args_copy, args);
		int needed = vsnprintf(nullptr, 0, format, args) + 1;
		std::unique_ptr<char[]> buffer(new char[needed]);
		vsnprintf(buffer.get(), needed, format, args_copy);
		va_end(args_copy);
		const char* levelStr = "";
		switch (level) {
			case INFO: levelStr = "INFO"; break;
			case LOG: levelStr = "LOG"; break;
			case ERROR: levelStr = "ERROR"; break;
		}
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		_file << "[" << std::put_time(&tm, "%d/%m/%Y - %H:%M:%S") << "] [" << levelStr
			  << "] " << buffer << std::endl;
	}

	void log(LogLevel level, const char* format, ...) {
		va_list args;
		va_start(args, format);
		vlog(level, format, args);
		va_end(args);
	}

private:
	Tintin_reporter();
	Tintin_reporter(const Tintin_reporter& tintin);
	Tintin_reporter& operator=(const Tintin_reporter& tintin);

	std::ofstream _file;
};

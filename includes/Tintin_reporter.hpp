#pragma once

#include "../includes/matt_daemon.hpp"

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
		static const size_t bufferSize = 1024;
		char buffer[bufferSize];
		vsnprintf(buffer, bufferSize, format, args);
		const char* levelStr = "";
		switch (level) {
			case INFO: levelStr = "INFO"; break;
			case LOG: levelStr = "LOG"; break;
			case ERROR: levelStr = "ERROR"; break;
		}
		// TODO: remove half of the lines when file too big
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
	std::ofstream _file;
};

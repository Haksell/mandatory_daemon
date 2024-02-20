#pragma once

#include "../includes/matt_daemon.hpp"

class Tintin_reporter {
public:
	Tintin_reporter(const std::string& filename) {
		// TODO: check truncation
		_file.open(filename, std::ios::out | std::ios::app);
		if (!_file.is_open()) fileError("open", filename.c_str());
	}

	~Tintin_reporter() {
		if (_file.is_open()) _file.close();
	}

	void log(const std::string& message) {
		// TODO: remove half of the lines when file too big
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		_file << "[" << std::put_time(&tm, "%d/%m/%Y - %H:%M:%S") << "] " << message
			  << std::endl;
	}

private:
	std::ofstream _file;
};

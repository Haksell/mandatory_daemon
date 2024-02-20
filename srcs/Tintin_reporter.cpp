#include "../includes/matt_daemon.hpp"

Tintin_reporter::Tintin_reporter(const std::string& filename) {
	// TODO: check truncation
	_file.open(filename, std::ios::out | std::ios::app);
	if (!_file.is_open()) fileError("open", filename.c_str());
}

Tintin_reporter::~Tintin_reporter() {
	if (_file.is_open()) _file.close();
}

void Tintin_reporter::log(const std::string& message) {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	_file << "[" << std::put_time(&tm, "%d/%m/%Y - %H:%M:%S") << "] " << message
		  << std::endl;
}

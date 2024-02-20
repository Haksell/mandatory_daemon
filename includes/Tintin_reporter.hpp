#pragma once

#include "../includes/matt_daemon.hpp"

// TODO: remove half of the lines when file too big

class Tintin_reporter {
public:
	Tintin_reporter(const std::string& filename);
	~Tintin_reporter();
	void log(const std::string& message);

private:
	std::ofstream _file;
};

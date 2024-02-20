#include "../includes/matt_daemon.hpp"

void fileError(const char* action, const char* filename) {
	std::cerr << RED << "Failed to " << action << " file " << filename << std::endl;
	panic("Failed to open file %s", filename);
}

void syscall(int returnValue, const char* funcName) {
	if (returnValue < 0) throw SystemError(funcName);
}

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ERR, format, args);
	va_end(args);
	std::exit(EXIT_FAILURE);
}
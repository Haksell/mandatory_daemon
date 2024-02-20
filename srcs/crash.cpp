#include "../includes/matt_daemon.hpp"

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
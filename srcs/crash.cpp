#include "../includes/matt_daemon.hpp"
#include <cstring>

extern Tintin_reporter logger;

void cleanup() {
	unlink(PID_FILE);
	unlink(LOCK_FILE);
}

void fileError(const char* action, const char* filename) {
	std::cerr << RED << "Failed to " << action << " file " << filename << std::endl;
	cleanup();
	panic("Failed to open file %s", filename);
}

void syscall(int returnValue, const char* funcName) {
	if (returnValue < 0) {
		logger.log(LogLevel::ERROR, "%s: %s", funcName, strerror(errno));
		throw SystemError();
	}
}

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	logger.vlog(LogLevel::ERROR, format, args); // TODO: ERROR
	va_end(args);
	throw SystemError();
}
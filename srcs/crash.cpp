#include "../includes/matt_daemon.hpp"

extern Tintin_reporter logger;
extern int fdLock;
extern int fdPid;

void cleanup() {
	close(fdPid);
	unlink(PID_FILE);
	close(fdLock);
	unlink(LOCK_FILE);
}

void exitWithLog(int returnValue) {
	logger.log(LogLevel::INFO, "Stopping " DAEMON_NAME);
	std::exit(returnValue);
}

void fileError(const char* action, const char* filename) {
	std::cerr << RED << "Failed to " << action << " file " << filename << RESET
			  << std::endl;
	cleanup();
	panic("Failed to open file %s", filename);
}

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	logger.vlog(LogLevel::ERROR, format, args);
	va_end(args);
	exitWithLog(EXIT_FAILURE);
}

void syscall(int returnValue, const char* funcName) {
	if (returnValue < 0) {
		logger.log(LogLevel::ERROR, "%s: %s", funcName, strerror(errno));
		exitWithLog(EXIT_FAILURE);
	}
}
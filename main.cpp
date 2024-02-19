#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

#define DAEMON_NAME "Matt_daemon"
#define SLEEP_INTERVAL 5000

#define LOCK_FILE "/run/matt_daemon.lock"
#define PID_FILE "/run/matt_daemon.pid"

static void handleSigchld(int sig) {
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
}

static void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ERR, format, args);
	va_end(args);
	std::exit(EXIT_FAILURE);
}

static void becomeChild() {
	pid_t pid = fork();
	if (pid < 0) panic("fork() failed");
	if (pid > 0) std::exit(EXIT_SUCCESS);
}

static void redirectToDevNull(int devNull, int fd) {
	close(fd);
	dup2(devNull, fd);
}

static int createLockFile(const char* filename) {
	int fd = open(filename, O_RDWR | O_CREAT, 0640);
	if (fd < 0) panic("Failed to open file %s", filename);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0) panic("Failed to lock file %s", filename);
	return fd;
}

static int createPidFile(const char* filename) {
	static size_t bufferSize = 32;
	int fd = createLockFile(filename);
	char buf[bufferSize];
	snprintf(buf, bufferSize, "%ld\n", (long)getpid());
	write(fd, buf, strlen(buf));
	return fd;
}

static void daemonize() {
	becomeChild();
	if (setsid() < 0) panic("setsid() failed");
	becomeChild();
	openlog(DAEMON_NAME, LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "Successfully started " DAEMON_NAME);
	umask(0);
	if (chdir("/") < 0) panic("Failed to change directory to /");
	signal(SIGCHLD, handleSigchld);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	int devNull = open("/dev/null", O_RDWR);
	redirectToDevNull(devNull, STDIN_FILENO);
	redirectToDevNull(devNull, STDOUT_FILENO);
	redirectToDevNull(devNull, STDERR_FILENO);
	close(devNull);
	createLockFile(LOCK_FILE);
	createPidFile(PID_FILE);
}

static void loop() {
	static int i = 0;
	syslog(LOG_NOTICE, "Running " DAEMON_NAME "... %d", i);
	++i;
}

int main(void) {
	daemonize();
	while (true) {
		loop();
		syslog(LOG_NOTICE, "before sleep");
		usleep(SLEEP_INTERVAL);
		syslog(LOG_NOTICE, "after sleep");
	}
	std::exit(EXIT_SUCCESS);
}
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

#define DAEMON_NAME "Matt_daemon"

// TODO: Disassociate from its process group (usually a shell), to insulate
// itself from signals (such as HUP) sent to the process group
// TODO: Ignore all terminal I/O signals
// TODO: Disassociate from the control terminal (and take steps not to reacquire
// one)
// TODO: Disassociate from the control terminal (and take steps not to reacquire
// one)

static void do_heartbeat() {
	// TODO: implement processing code to be performed on each heartbeat
}

static void daemonize() {
	pid_t pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS);

	umask(0);

	openlog(DAEMON_NAME, LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "Successfully started " DAEMON_NAME);

	if (setsid() < 0) {
		syslog(LOG_ERR, "Could not generate session ID for child process");
		exit(EXIT_FAILURE);
	}

	if (chdir("/") < 0) {
		syslog(LOG_ERR, "Could not change working directory to /");
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

// TODO: understand why it is not recommended to allow argc, argv
int main(void) {
	daemonize();

	const int SLEEP_INTERVAL = 5;
	while (true) {
		do_heartbeat();
		sleep(SLEEP_INTERVAL);
	}

	syslog(LOG_NOTICE, "Stopping " DAEMON_NAME);
	closelog();
	exit(EXIT_SUCCESS);
}
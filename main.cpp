#include <csignal>
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
#include <syslog.h>
#include <unistd.h>
#include <vector>

#define DAEMON_NAME "Matt_daemon"
#define LOCK_FILE "/var/run/matt_daemon.lock"
#define SLEEP_INTERVAL 500000

// TODO: Ignore all terminal I/O signals
// TODO: Disassociate from the control terminal (and take steps not to reacquire
// one)
// TODO: Handle any SIGCLD signals
// TODO: matt_daemon.lock (using flock?) (in what folder?)

static void becomeChild() {
	pid_t pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS);
}

static void daemonize() {
	becomeChild();
	umask(0); // TODO: verify the u\value is correct

	openlog(DAEMON_NAME, LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "Successfully started " DAEMON_NAME);

	if (setsid() < 0) {
		syslog(LOG_ERR, "Could not generate session ID for child process");
		exit(EXIT_FAILURE);
	}

	becomeChild();
	int lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
	if (lock_fd < 0) {
		std::cerr << "open failed" << std::endl;
		syslog(LOG_ERR, "Could not open lock file " LOCK_FILE);
		exit(EXIT_FAILURE);
	}
	if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
		std::cerr << "flock failed" << std::endl;
		syslog(LOG_ERR,
			   "Could not lock " LOCK_FILE ", another instance may be running");
		exit(EXIT_FAILURE);
	}

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	if (chdir("/") < 0) {
		syslog(LOG_ERR, "Could not change working directory to /");
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

static void do_heartbeat() { syslog(LOG_NOTICE, "Running " DAEMON_NAME "..."); }

int main(void) {
	daemonize();
	while (true) {
		do_heartbeat();
		usleep(SLEEP_INTERVAL);
	}
	syslog(LOG_NOTICE, "Stopping " DAEMON_NAME);
	closelog();
	exit(EXIT_SUCCESS);
}
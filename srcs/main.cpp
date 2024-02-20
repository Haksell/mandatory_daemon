#include "../includes/matt_daemon.hpp"

Tintin_reporter logger(LOG_FILE);
Server server(PORT);
int fdLock = -1;
int fdPid = -1;

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
	if (fd < 0) fileError("open", filename);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0) fileError("lock", filename);
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

static void logSignal(int sig) {
	static const std::unordered_map<int, std::string> signalNames = {
		{SIGHUP, "SIGHUP"},	  {SIGINT, "SIGINT"},	  {SIGQUIT, "SIGQUIT"},
		{SIGILL, "SIGILL"},	  {SIGTRAP, "SIGTRAP"},	  {SIGABRT, "SIGABRT"},
		{SIGBUS, "SIGBUS"},	  {SIGFPE, "SIGFPE"},	  {SIGUSR1, "SIGUSR1"},
		{SIGSEGV, "SIGSEGV"}, {SIGUSR2, "SIGUSR2"},	  {SIGPIPE, "SIGPIPE"},
		{SIGALRM, "SIGALRM"}, {SIGTERM, "SIGTERM"},	  {SIGCHLD, "SIGCHLD"},
		{SIGCONT, "SIGCONT"}, {SIGSTOP, "SIGSTOP"},	  {SIGTSTP, "SIGTSTP"},
		{SIGTTIN, "SIGTTIN"}, {SIGTTOU, "SIGTTOU"},	  {SIGURG, "SIGURG"},
		{SIGXCPU, "SIGXCPU"}, {SIGXFSZ, "SIGXFSZ"},	  {SIGVTALRM, "SIGVTALRM"},
		{SIGPROF, "SIGPROF"}, {SIGWINCH, "SIGWINCH"}, {SIGIO, "SIGIO"},
		{SIGPWR, "SIGPWR"},

	};
	auto it = signalNames.find(sig);
	logger.log(LogLevel::INFO, "Received signal %d (%s)", sig,
			   it == signalNames.end() ? "UNKNOWN" : it->second.c_str());
}

static void handleRemainingSignals(int sig) {
	logSignal(sig);
	logger.log(LogLevel::INFO, "Stopping " DAEMON_NAME);
	std::exit(EXIT_SUCCESS);
}

static void handleSIGCHLD(int sig) {
	logSignal(sig);
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
}

static void setupRemainingSignals() {
	signal(SIGHUP, handleRemainingSignals);
	signal(SIGINT, handleRemainingSignals);
	signal(SIGQUIT, handleRemainingSignals);
	signal(SIGILL, handleRemainingSignals);
	signal(SIGTRAP, handleRemainingSignals);
	signal(SIGABRT, handleRemainingSignals);
	signal(SIGBUS, handleRemainingSignals);
	signal(SIGFPE, handleRemainingSignals);
	signal(SIGUSR1, handleRemainingSignals);
	signal(SIGSEGV, handleRemainingSignals);
	signal(SIGUSR2, handleRemainingSignals);
	signal(SIGPIPE, handleRemainingSignals);
	signal(SIGALRM, handleRemainingSignals);
	signal(SIGTERM, handleRemainingSignals);
	signal(SIGCONT, handleRemainingSignals);
	signal(SIGURG, handleRemainingSignals);
	signal(SIGXCPU, handleRemainingSignals);
	signal(SIGXFSZ, handleRemainingSignals);
	signal(SIGVTALRM, handleRemainingSignals);
	signal(SIGPROF, handleRemainingSignals);
	signal(SIGIO, handleRemainingSignals);
	signal(SIGPWR, handleRemainingSignals);
}

static void daemonize() {
	becomeChild();
	if (setsid() < 0) panic("setsid() failed");
	becomeChild();
	atexit(cleanup);
	logger.log(LogLevel::INFO, "Successfully started " DAEMON_NAME);
	umask(0);
	if (chdir("/") < 0) panic("Failed to change directory to /");
	int devNull = open("/dev/null", O_RDWR);
	redirectToDevNull(devNull, STDIN_FILENO);
	redirectToDevNull(devNull, STDOUT_FILENO);
	redirectToDevNull(devNull, STDERR_FILENO);
	close(devNull);
	signal(SIGCHLD, handleSIGCHLD);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGWINCH, SIG_IGN);
}

int main(void) {
	int exitValue = EXIT_FAILURE;
	try {
		fdLock = createLockFile(LOCK_FILE);
		fdPid = createPidFile(PID_FILE);
		if (!DEBUG) daemonize();
		atexit(cleanup);
		setupRemainingSignals();
		server.init();
		server.loop();
	} catch (SystemError&) {
	} catch (TerminateSuccess&) {
		logger.log(LogLevel::INFO, "Stopping " DAEMON_NAME);
		exitValue = EXIT_SUCCESS;
	} catch (const std::exception& e) {
		logger.log(LogLevel::ERROR, "Unexpected exception: %s", e.what());
	}
	std::exit(exitValue);
}

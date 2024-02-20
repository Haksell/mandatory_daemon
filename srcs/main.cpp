#include "../includes/matt_daemon.hpp"

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
	syslog(LOG_NOTICE, "Received signal %d (%s)", sig,
		   it == signalNames.end() ? "UNKNOWN" : it->second.c_str());
}

static void handleRemainingSignals(int sig) {
	logSignal(sig);
	syslog(LOG_NOTICE, "Stopping " DAEMON_NAME);
	std::exit(EXIT_SUCCESS);
}

static void handleSIGCHLD(int sig) {
	logSignal(sig);
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
}

static void setupSignalHandlers() {
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
	signal(SIGCHLD, handleSIGCHLD);
	signal(SIGCONT, handleRemainingSignals);
	signal(SIGSTOP, handleRemainingSignals);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGURG, handleRemainingSignals);
	signal(SIGXCPU, handleRemainingSignals);
	signal(SIGXFSZ, handleRemainingSignals);
	signal(SIGVTALRM, handleRemainingSignals);
	signal(SIGPROF, handleRemainingSignals);
	signal(SIGWINCH, SIG_IGN);
	signal(SIGIO, handleRemainingSignals);
	signal(SIGPWR, handleRemainingSignals);
}

static void cleanup() {
	unlink(PID_FILE);
	unlink(LOCK_FILE);
	syslog(LOG_NOTICE, "cleanup()");
	closelog();
}

static void daemonize() {
	becomeChild();
	if (setsid() < 0) panic("setsid() failed");
	becomeChild();
	atexit(cleanup);
	openlog(DAEMON_NAME, LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "Successfully started " DAEMON_NAME);
	umask(0);
	if (chdir("/") < 0) panic("Failed to change directory to /");
	int devNull = open("/dev/null", O_RDWR);
	redirectToDevNull(devNull, STDIN_FILENO);
	redirectToDevNull(devNull, STDOUT_FILENO);
	redirectToDevNull(devNull, STDERR_FILENO);
	close(devNull);
	createLockFile(LOCK_FILE);
	createPidFile(PID_FILE);
	setupSignalHandlers();
}

int main(void) {
	daemonize();
	Server server(PORT, LOG_FILE);
	try {
		server.init();
		server.loop();
		return EXIT_SUCCESS;
	} catch (const SystemError& e) {
		std::perror(e.funcName);
		return EXIT_FAILURE;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::exit(EXIT_SUCCESS);
}

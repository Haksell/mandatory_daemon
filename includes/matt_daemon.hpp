#pragma once

// TODO: remove unused libs
#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

#define DAEMON_NAME "Matt_daemon"
#define SLEEP_INTERVAL 1000000

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define PID_FILE "/run/matt_daemon.pid"
#define LOG_FILE "/var/log/matt_daemon.log"

#define BACKLOG 128
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024
#define PORT 4242

class SystemError : public std::runtime_error {
public:
	explicit SystemError(const char* funcName)
		: std::runtime_error(funcName), funcName(funcName) {}

	virtual ~SystemError() throw() {}

	const char* funcName;
};

template <typename T>
void deleteVector(std::vector<T*>* vec) {
	for (typename std::vector<T*>::iterator it = vec->begin(); it != vec->end(); it++)
		delete *it;
	vec->clear();
}

void cleanup();
void fileError(const char* action, const char* filename);
void panic(const char* format, ...);
void syscall(int returnValue, const char* funcName);
std::string trimNewlines(const std::string& str);

// TODO: Coplien classes

class Tintin_reporter;
class Client;
class Server;

#include "Tintin_reporter.hpp"

#include "Client.hpp"

#include "Server.hpp"

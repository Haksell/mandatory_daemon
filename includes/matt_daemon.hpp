#pragma once

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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#ifndef DEBUG
#define DEBUG false
#endif

#if DEBUG
#define LOCK_FILE "/tmp/matt_daemon.lock"
#define LOG_FILE "/tmp/matt_daemon.log"
#else
#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define LOG_FILE "/var/log/matt_daemon.log"
#endif

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

#define DAEMON_NAME "Matt_daemon"

#define BACKLOG 8
#define MAX_CLIENTS 8
#define PORT 4242

template <typename T>
void deleteVector(std::vector<T*>* vec) {
	for (typename std::vector<T*>::iterator it = vec->begin(); it != vec->end(); it++)
		delete *it;
	vec->clear();
}

template <typename T>
std::string toString(T x) {
	std::stringstream ss;
	ss << x;
	return ss.str();
}

// crash.cpp
void cleanup();
void exitWithLog(int returnValue);
void fileError(const char* action, const char* filename);
void panic(const char* format, ...);
void syscall(int returnValue, const char* funcName);

// utils.cpp
std::string toLowerCase(const std::string& str);
std::string trimNewlines(const std::string& str);

// TODO: Coplien classes

class Tintin_reporter;
class Client;
class Server;

#include "Tintin_reporter.hpp"

#include "Client.hpp"

#include "Server.hpp"

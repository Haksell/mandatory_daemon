#include "../includes/matt_daemon.hpp"

static inline bool isNewline(int ch) { return ch == '\r' || ch == '\n'; }

std::string trimNewlines(const std::string& str) {
	auto begin = std::find_if_not(str.begin(), str.end(), isNewline);
	auto end = std::find_if_not(str.rbegin(), str.rend(), isNewline).base();
	if (begin >= end) return "";
	return std::string(begin, end);
}
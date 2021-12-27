#include "log.h"
#include <cstdio>
#include <cstdarg>

void Log::SetLevel(Level level) {
	m_LogLevel = level;
}

void Log::Error(const char *fmt, ...) {
	if (m_LogLevel >= LevelError) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "[ERROR  ]: %s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

void Log::Warning(const char *fmt, ...) {
	if (m_LogLevel >= LevelWarning) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "[WARNING]: %s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

void Log::Info(const char *fmt, ...) {
	if (m_LogLevel >= LevelInfo) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "[INFO   ]: %s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

void Log::Debug(const char *fmt, ...) {
	if (m_LogLevel >= LevelDebug) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "[DEBUG  ]: %s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

void Log::Trace(const char *fmt, ...) {
	if (m_LogLevel >= LevelTrace) {
		char msg[8192];
		snprintf(msg, sizeof(msg), "[TRACE  ]: %s", fmt);
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, msg, va);
		va_end(va);
	}
}

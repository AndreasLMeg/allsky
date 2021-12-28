#ifndef LOG_H
#define LOG_H

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

class Log 
{
	public:
		enum Level
		{
			LevelError = 0,
			LevelWarning,
			LevelInfo,
			LevelDebug,
			LevelTrace,
			LevelMax
		};
	private:
		static Level m_LogLevel;
	public:
		static void SetLevel(Level level);
		static void Error(const char *fmt, ...);
		static void Warning(const char *fmt, ...);
		static void Info(const char *fmt, ...);
		static void Debug(const char *fmt, ...);
		static void Trace(const char *fmt, ...);
};

#endif
#ifndef LOG_H
#define LOG_H

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
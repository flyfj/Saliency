#pragma once

class CmLog
{
public:	
	// A demo function to show how to use this class
	static void Demo();

	static void Set(const string& name, bool append, const string& dataSN = string());
	static void Set(bool logPrefix); 

	// Log information and show to the console
	static void Log(const string& msg);
	static void Log(const char* msg);
	static void LogLine(const char* format, ...);
	static void LogLine(WORD attribs, const char* format, ...);  //FOREGROUND_GREEN
	static void LogError(const char* format, ...);
	static void LogWarning(const char* format, ...);

	// Show information to the console but not log it
	static void LogProgress(const char* format, ...);

	// Get file storage
	static FileStorage& GetFS();

	// Clear log file
	static void LogClear(void);

private:
	static HANDLE m_hConsole;
	static string m_fileName;
	static CmLog* gLog;
	static bool m_LogPrefix;
	static string m_DataSN; // data save name

	static const int LOG_BUF_LEN = 1024;
	static DWORD gConsoleWritenLen; //Just for avoiding debug warning when writeConsole is called
	static char gLogBufferA[LOG_BUF_LEN];

	// Some time information about current line of log
	static void LogFilePrefix(FILE* file);
};

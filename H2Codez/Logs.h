#pragma once
#include "stdafx.h"
using namespace std;

class Logs
{
public:
	Logs(char* filename); //Constructor that creates and sets the Log name
	void Exit();      //close file
	void WriteLog(const char* line, ...); //Write to Log File

private:
	ofstream file;

};

namespace Debug
{
	void Start_Console();///AllocConsole		
};
extern Logs pLog;
extern Logs H2PCTool;


template <typename T>
inline T verify_output(T output, const char *expression, const char *func_name, const char* file, const int line)
{
	if (!output) {
		pLog.WriteLog("'%s' failed in '%s' at '%s:%d'!", func_name, expression, file, line);
		DWORD last_error = GetLastError();
		if (last_error)
		{
			LPWSTR messageBuffer = NULL;
			size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
			if (size) {
				pLog.WriteLog("Last error: '%ws'", messageBuffer);
				LocalFree(messageBuffer);
			} else {
				pLog.WriteLog("Converting error %d to string failed!", last_error);
			}
			SetLastError(0);
		}
	}
	return output;
}
#define LOG_CHECK(expression) \
	verify_output(expression, #expression, __FUNCTION__, __FILE__, __LINE__)
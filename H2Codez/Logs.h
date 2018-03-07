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
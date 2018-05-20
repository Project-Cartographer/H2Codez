#include "stdafx.h"



Logs::Logs(char* filename)
{
	
	file.open(filename); //open file
}


void Logs::WriteLog(const char* line, ...)
{
	
	SYSTEMTIME tt;
GetLocalTime(&tt);

	char buf[2048];	
	va_list myarg;

	//Creating the buffer 
	va_start(myarg,line);
	vsnprintf(buf,2048,line,myarg);
	va_end(myarg);

#if _DEBUG
	// print to console
	printf("Logs: %s\n", buf);
#endif

	//Lets now Write the buffer to our opened File

	file<<tt.wHour<<":"<<tt.wMinute<<":"<<tt.wSecond<<":"<<tt.wMilliseconds<<"\t";//Lets add time to log
	file<<buf<<endl; //Write to log
	
}

void Logs::Exit()
{
	file.flush();
	file.close();
}



void Debug::Start_Console()
{
	AllocConsole();
	SetConsoleTitleA("Debug Window");

	// out
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);

	// in
	FILE* pCin;
	freopen_s(&pCin, "CONIN$", "r", stdin);
}




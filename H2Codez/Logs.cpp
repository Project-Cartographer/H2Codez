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
	
	if(EnableDbgConsole)
	cout<<buf<<endl; //Write to Allocated Console

	//Lets now Write the buffer to our opened File

	file<<tt.wHour<<":"<<tt.wMinute<<":"<<tt.wSecond<<":"<<tt.wMilliseconds<<"\t";//Lets add time to log
	file<<buf<<endl; //Write to log
	
}

void Logs::Exit()
{
	file.flush();
	file.close();
}



VOID Debug::Start_Console()//AllocConsole	
{
	AllocConsole();
	SetConsoleTitleA("Debug Window");
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
}




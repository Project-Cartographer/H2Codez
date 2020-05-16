#include "Logs.h"
#include "h2codez.h"

Logs::Logs(char* filename, bool _console)
{
	file.open(filename); //open file
	console = is_debug_build() ? _console : _console && game.process_type != H2EK::H2Tool;
}


void Logs::WriteLog(const char* line, ...)
{
	
	SYSTEMTIME tt;
	GetLocalTime(&tt);

	char buf[0x2000];	
	va_list myarg;
	//Creating the buffer 
	va_start(myarg,line);
	vsnprintf(buf, ARRAYSIZE(buf), line, myarg);
	va_end(myarg);
	// print to console
	if (console)
		printf("Logs: %s\n", buf);

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
	if (game.process_type != H2EK::H2Tool)
	{
		AllocConsole();
		SetConsoleTitleA("Debug Window");
	}

	// out
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);

	// in
	FILE* pCin;
	freopen_s(&pCin, "CONIN$", "r", stdin);
}

Logs& getLogger()
{
	static Logs logger("H2Codez.log");
	return logger;
}

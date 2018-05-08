// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "DiscordInterface.h"
#include "SigScanning.h"
#include "Debug.h"

#pragma region declarations

DWORD g_threadID;
HMODULE g_hModule;

H2EK_Globals game;

Logs pLog("H2Codez.log");
Logs H2PCTool("H2PCTool.log");
Settings conf("H2Codez.conf");

BOOL EnableDbgConsole;

#pragma endregion

__declspec(dllexport) void IIDking_import(void) {};

BOOL APIENTRY DllMain( HMODULE hModule,
	                   DWORD  ul_reason_for_call,
	                   LPVOID lpReserved
	                  )
{
	switch (ul_reason_for_call)
	{

	case DLL_PROCESS_ATTACH:
#if _DEBUG
		Debug::Start_Console();
#endif
		g_hModule = hModule;
		if (!H2Toolz::Init())
			std::exit(0);     
	    break;
	
	case DLL_PROCESS_DETACH:
		std::string cmd = GetCommandLineA();
		if (!g_process_crashed && game.process_type == H2EK::H2Tool 
				&& cmd.find("pause_after_run") != string::npos) {
			std::cout << "Press any key to exit tool...." << std::endl;
			std::cin.get();
		}
		break;
	}
	return TRUE;
}

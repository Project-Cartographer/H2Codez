// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "SigScanning.h"

#pragma region declarations

DWORD g_threadID;
HMODULE g_hModule;

Debug Dbg;
H2EK_Globals game;

Logs pLog= Logs("H2Codez.log");
Logs H2PCTool = Logs("H2PCTool.log");


BOOL EnableDbgConsole;

#pragma endregion


/* dead code
//Enable Hooking System
void APPLYHOOKS()
{ 
  pLog.WriteLog("Hooks:: Initializing Hooking System....");   
  //CreateThread(0, 0, (LPTHREAD_START_ROUTINE)H2Hooks, 0, 0, 0);
  pLog.WriteLog("Hooks:: InProgress");

}

//Remove Hooking System
void DisEngageHOOKS()
{
    
DetourTransactionBegin();



DetourTransactionCommit();
pLog.WriteLog("\n\n***H2 Codez ShutDown****");
pLog.Exit();

ExitProcess(true);
}



	
DWORD WINAPI H2EK_Initz(LPVOID)
{
	if (GetModuleHandleW(L"H2Tool.exe"))
	{
		game.base = GetModuleHandleW(L"H2Tool.exe");
		game.process_type = H2EK::H2Tool;
		tool->Initialize();
	}
	else 
		if (GetModuleHandleW(L"H2Sapien.exe"))
		{
			game.base = GetModuleHandleW(L"H2Sapien.exe");
			game.process_type = H2EK::H2Sapien;
		}
	return 0;
}
*/



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
		Discord_Shutdown();
		std::string cmd = GetCommandLineA();
		if (cmd.find("pause_after_run") != string::npos)
			std::cin.get();
		Discord_Shutdown();
		break;
	}
	return TRUE;
}

// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "SigScanning.h"

#pragma region declarations

DWORD g_threadID;

Debug Dbg;
H2EK_Globals game;

Logs pLog= Logs("H2Codez.log");
Logs H2PCTool = Logs("H2PCTool.log");


BOOL EnableDbgConsole;



#define PI 3.14159265
#define degreesToRadians(angleDegrees) (angleDegrees * PI / 180.0)
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / PI)
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

DWORD WINAPI SigScanning(LPVOID)
{
	EnableDbgConsole = TRUE;
	Dbg.Start_Console();
	pLog.WriteLog("\n*** H2 Codez***\nBETA v1.5\n :) \nInjected Successfully");
	DWORD ADDY = FindPattern("\x83\x3D\x00\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\x89\x3D\x00\x00\x00\x00\x75\x73\x6A\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00", "xx?????xx????????xx????xxxxx????x????");
	pLog.WriteLog("Address of Function : %X", ADDY);
	return 0;
}

__declspec(dllexport) void IIDking_import(void) {};

BOOL APIENTRY DllMain( HMODULE hModule,
	                   DWORD  ul_reason_for_call,
	                   LPVOID lpReserved
	                  )
{
	switch (ul_reason_for_call)
	{

	case DLL_PROCESS_ATTACH:
		if (!H2Toolz::Init())
			std::exit(0);     
	    break;
	
	case DLL_PROCESS_DETACH:
        
	   // DisEngageHOOKS();
		break;
	}
	return TRUE;
}

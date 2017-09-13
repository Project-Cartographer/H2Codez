#pragma once
#include "stdafx.h"
#include <Psapi.h>

//Whats this file for?
//I use Signature Scanning here to find similar opcodes b/w Halo 2 and The Editing Kit package
//Can also be used to compare signatures of other processes

MODULEINFO GetModeuleInfo(char* szModule)
{
	MODULEINFO p = { 0 };
	HMODULE q = GetModuleHandleA(szModule);
	if (q == 0)
	{
		pLog.WriteLog("Process Not Found ");
		return p;
	}
	GetModuleInformation(GetCurrentProcess(), q, &p, sizeof(MODULEINFO));
	return p;
}

DWORD FindPattern(char *szPattern, char *szMask)
{
	char* ProcName = "halo2.exe";
	//char* ProcName = "H2Sapien.exe";
	//char* ProcName = "H2Sapien.exe";
	// Get the current process information
	MODULEINFO mInfo = GetModeuleInfo(ProcName);
	// Find the base address 
	DWORD dwBase = (DWORD)mInfo.lpBaseOfDll;
	DWORD dwSize = (DWORD)mInfo.SizeOfImage;
	// Get the pattern length
	DWORD dwPatternLength = (DWORD)strlen(szMask);
	// Loop through all the process
	for (DWORD i = 0; i < dwSize - dwPatternLength; i++)
	{
		bool bFound = true;
		// Loop through the pattern caracters
		for (DWORD j = 0; j < dwPatternLength; j++)
			bFound &= szMask[j] == '?' || szPattern[j] == *(char*)(dwBase + i + j);

		// If found return the current address
		if (bFound)
			return dwBase + i;
	}
	// Return null
	return NULL;
}
#pragma once
#include <string>
#include "h2codez.h"

namespace process
{
	bool newInstance();
	std::wstring GetExeDirectoryWide();
	std::string GetExeDirectoryNarrow();
	DWORD GetModuleFileNameA(
		HMODULE hModule,
		LPSTR lpFilename,
		DWORD nSize
	);
	DWORD GetModuleFileNameW(
		HMODULE hModule,
		LPWSTR  lpFilename,
		DWORD   nSize
	);
}

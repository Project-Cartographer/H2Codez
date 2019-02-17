#pragma once
#include <string>
#include "h2codez.h"

namespace process
{
	bool newInstance(std::wstring command_line = L"", HANDLE *new_process_handle = nullptr, const wchar_t *current_directory = nullptr);
	struct fork_info
	{
		bool is_parent;
		DWORD pid_other;
		HANDLE handle_other;
	};
	/* 
		Creates a fork using an undocumented windows API, Returns success
		Doesn't reinitiate the csr connection so a lot of more "complex" functions will crash  
	*/
	bool fork(fork_info &info, bool create_suspended = false);
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

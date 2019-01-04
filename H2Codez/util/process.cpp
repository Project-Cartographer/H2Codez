#include "process.h"
#include "H2Sapien\H2Sapien.h"

bool process::newInstance()
{
	TCHAR exePath[MAX_PATH];
	if (!LOG_CHECK(GetModuleFileName(NULL, exePath, MAX_PATH)))
		return false;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	return LOG_CHECK(CreateProcess(exePath, nullptr, nullptr, nullptr, false, INHERIT_PARENT_AFFINITY, nullptr, nullptr, &si, &pi));
}

std::wstring process::GetExeDirectoryWide()
{
	wchar_t main_exe_dir[MAX_PATH];
	GetModuleFileNameW(NULL, main_exe_dir, sizeof(main_exe_dir));
	size_t path_len = wcsnlen_s(main_exe_dir, sizeof(main_exe_dir));
	if (main_exe_dir[path_len - 1] == L'\\')
		main_exe_dir[path_len - 1] = NULL;
	wchar_t *last_part = wcsrchr(main_exe_dir, L'\\');
	if (last_part)
		*last_part = NULL;
	return main_exe_dir;
}

std::string process::GetExeDirectoryNarrow()
{
	char main_exe_dir[MAX_PATH];
	GetModuleFileNameA(NULL, main_exe_dir, sizeof(main_exe_dir));
	size_t path_len = strnlen_s(main_exe_dir, sizeof(main_exe_dir));
	if (main_exe_dir[path_len - 1] == L'\\')
		main_exe_dir[path_len - 1] = '\0';
	char *last_part = strrchr(main_exe_dir, L'\\');
	if (last_part)
		*last_part = '\0';
	return main_exe_dir;
}

DWORD process::GetModuleFileNameA(
	HMODULE hModule,
	LPSTR   lpFilename,
	DWORD   nSize
)
{
	if (game.process_type == H2EK::H2Sapien)
	{
		return H2SapienPatches::GetModuleFileNameA_org(hModule, lpFilename, nSize);
	}
	return ::GetModuleFileNameA(hModule, lpFilename, nSize);
}

DWORD process::GetModuleFileNameW(
	HMODULE hModule,
	LPWSTR  lpFilename,
	DWORD   nSize
)
{
	if (game.process_type == H2EK::H2Sapien)
	{
		return H2SapienPatches::GetModuleFileNameW_org(hModule, lpFilename, nSize);
	}
	return ::GetModuleFileNameW(hModule, lpFilename, nSize);
}

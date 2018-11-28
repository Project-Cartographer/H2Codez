#include "process.h"
#include "h2codez.h"

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
		main_exe_dir[path_len - 1] = NULL;
	char *last_part = strrchr(main_exe_dir, L'\\');
	if (last_part)
		*last_part = NULL;
	return main_exe_dir;
}

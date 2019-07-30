#include "process.h"
#include "H2Sapien\H2Sapien.h"

bool process::newInstance(std::wstring command_line, HANDLE *new_process_handle, const wchar_t *current_directory)
{
	LOG_FUNC("Command line: %ws", command_line.c_str());
	WCHAR exePath[MAX_PATH];
	if (!LOG_CHECK(GetModuleFileNameW(NULL, exePath, MAX_PATH)))
		return false;

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	// add the exe name to stop C/C++ programs from freaking out
	command_line = std::wstring(L"\"") + exePath + L"\" " + command_line;
	// the const_cast is technically non-standard compliant but it's fine
	auto success = LOG_CHECK(CreateProcessW(exePath, const_cast<wchar_t*>(command_line.c_str()), nullptr, nullptr, false, INHERIT_PARENT_AFFINITY | CREATE_NEW_CONSOLE, nullptr, current_directory, &si, &pi));

	if (success && new_process_handle)
		*new_process_handle = pi.hProcess;

	return success;
}

typedef LONG NTSTATUS, *PNTSTATUS;

typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
}CLIENT_ID, *PCLIENT_ID;

typedef struct _SECTION_IMAGE_INFORMATION
{
	PVOID TransferAddress;
	ULONG ZeroBits;
	SIZE_T MaximumStackSize;
	SIZE_T CommittedStackSize;
	ULONG SubSystemType;
	union
	{
		struct
		{
			USHORT SubSystemMinorVersion;
			USHORT SubSystemMajorVersion;
		};
		ULONG SubSystemVersion;
	};
	ULONG GpValue;
	USHORT ImageCharacteristics;
	USHORT DllCharacteristics;
	USHORT Machine;
	BOOLEAN ImageContainsCode;
	union
	{
		UCHAR ImageFlags;
		struct
		{
			UCHAR ComPlusNativeReady : 1;
			UCHAR ComPlusILOnly : 1;
			UCHAR ImageDynamicallyRelocated : 1;
			UCHAR ImageMappedFlat : 1;
			UCHAR Reserved : 4;
		};
	};
	ULONG LoaderFlags;
	ULONG ImageFileSize;
	ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;


typedef struct _RTL_USER_PROCESS_INFORMATION
{
	ULONG Length;
	HANDLE Process;
	HANDLE Thread;
	CLIENT_ID ClientId;
	SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

typedef
NTSTATUS
NTAPI
RtlCloneUserProcess(
	__in ULONG ProcessFlags,
	__in_opt PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	__in_opt PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	__in_opt HANDLE DebugPort,
	__out PRTL_USER_PROCESS_INFORMATION ProcessInformation
);

#define RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED 0x00000001
#define RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES 0x00000002
#define RTL_CLONE_PROCESS_FLAGS_NO_SYNCHRONIZE 0x00000004 // don't update synchronization objects

#define RTL_CLONE_PARENT				0
#define RTL_CLONE_CHILD					297

SYSTEMTIME fake_time;
VOID WINAPI GetLocalTimeHook(
	_Out_ LPSYSTEMTIME lpSystemTime
)
{
	if (lpSystemTime)
	{
		*lpSystemTime = fake_time;
	}
}

bool process::fork(fork_info &info, bool create_suspended)
{
	GetLocalTime(&fake_time); // get time so we can 
	info.pid_other = GetCurrentProcessId();
	auto ntdll = LOG_CHECK(GetModuleHandleW(L"ntdll"));
	auto clone_user_process = (RtlCloneUserProcess*)LOG_CHECK(GetProcAddress(ntdll, "RtlCloneUserProcess"));

	if (!clone_user_process)
		return false;

	RTL_USER_PROCESS_INFORMATION process_info;
	auto clone_result = clone_user_process(RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES | RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED, NULL, NULL, NULL, &process_info);

	if (clone_result == RTL_CLONE_PARENT)
	{
		info.pid_other = GetProcessId(process_info.Process);
		info.handle_other = process_info.Process;
		info.is_parent = true;

		if (!create_suspended)
			ResumeThread(process_info.Thread);
		CloseHandle(process_info.Thread);

		return true;
	}
	else if (clone_result == RTL_CLONE_CHILD)
	{
		// Replace GetLocalTime to fix crash caused by invalid csr data
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		auto GetLocalTimeOrg = GetLocalTime;
		DetourAttach(&(PVOID&)GetLocalTimeOrg, GetLocalTimeHook);
		DetourTransactionCommit();

		// reinit console
		FreeConsole();
		AllocConsole();
		info.is_parent = false;
		info.handle_other = LOG_CHECK(OpenProcess(PROCESS_QUERY_INFORMATION | READ_CONTROL | SYNCHRONIZE | PROCESS_VM_READ, FALSE, info.pid_other));
		return true;
	}
	return false;
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

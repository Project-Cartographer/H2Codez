#pragma once
#include <string>

namespace process
{
	bool newInstance();
	std::wstring GetExeDirectoryWide();
	std::string GetExeDirectoryNarrow();
}
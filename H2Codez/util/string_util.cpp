#include "string_util.h"
#include "process.h"

std::string get_full_tag_path(const std::string &tag_path)
{
	const std::string tags_folder = process::GetExeDirectoryNarrow() + "\\tags\\";
	if (strstr(tag_path.c_str(), "\\tags\\"))
	{
		return tag_path;
	} else {
		return tags_folder + tag_path;
	}
}

std::wstring utf8_to_utf16(const char* source, size_t source_len) {
	const int size = MultiByteToWideChar(CP_UTF8, 0, source, source_len, NULL, 0);
	if (LOG_CHECK(size < 0))
		return L"INVALID UTF-8";
	std::wstring result;
	result.resize(size + 10);
	ASSERT_CHECK(MultiByteToWideChar(CP_UTF8, 0, source, source_len, &result[0], result.size()) < 0);
	return result;
}

std::string utf16_to_utf8(const wchar_t* source, size_t source_len) {
	const int size = WideCharToMultiByte(CP_UTF8, 0, source, source_len, NULL, 0, NULL, NULL);
	if (LOG_CHECK(size < 0))
		return "INVALID UTF-8";
	std::string result;
	result.resize(size + 10);
	ASSERT_CHECK(WideCharToMultiByte(CP_UTF8, 0, source, source_len, &result[0], result.size(), NULL, NULL) < 0);
	return result;
}

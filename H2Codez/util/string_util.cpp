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
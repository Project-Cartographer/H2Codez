#include "TagUpdate.h"
#include "../stdafx.h"
#include "../util/FileWatcher.h"
#include "../Common/H2EKCommon.h"
#include <Shlobj.h>
#include <shlwapi.h>
#include "../Common/tag_group_names.h"

class UpdateListener : public FW::FileWatchListener
{
public:
	UpdateListener() {}
	void handleFileAction(FW::WatchID watchid, const std::string& dir, const std::string& filename,
		FW::Action action)
	{
		typedef int (__cdecl *tag_reload)(int tag_group, const char *tag_name);
		auto tag_reload_impl = reinterpret_cast<tag_reload>(0x004B5A90);
		void *sbsp_ptr = *reinterpret_cast<void**>(0xA9CA74);

		std::cout << "DIR (" << dir + ") FILE (" + filename + ") has event " << action << std::endl;
		auto file_ext_pos = filename.find_last_of('.');
		if (action == FW::Action::Modified && file_ext_pos != string::npos
			&& file_ext_pos + 1 < filename.size() && sbsp_ptr != NULL)
		{
			std::string file_ext = filename.substr(file_ext_pos + 1);
			std::string tag_name = filename.substr(0, file_ext_pos);

			auto tag_group = string_to_tag_group(file_ext);
			if (tag_group == 0xFFFF) // not a tag
				return;
			std::cout << "Reloading tag: \"" << tag_name << "\" type: \"" << file_ext << "\"" << std::endl;
			tag_reload_impl(tag_group, tag_name.c_str());
		}
	}
};

DWORD WINAPI TagSyncUpdate(
	_In_ LPVOID lpParameter
)
{
	UpdateListener listener;
	FW::FileWatcher fileWatcher;

	char h2_docs_folder[0x200];
	SHGetFolderPathA(0, CSIDL_PERSONAL, 0, 0, h2_docs_folder);// get user documents folder
	PathAppendA(h2_docs_folder, "Halo 2\\Tags");

	fileWatcher.addWatch(H2CommonPatches::GetExeDirectoryNarrow() + "\\tags", &listener, true);
	fileWatcher.addWatch(h2_docs_folder, &listener, true);
	while (true)
	{
		fileWatcher.update();
		Sleep(1 * 1000);
	}
}


void H2SapienPatches::StartTagSync()
{
	if (conf.getBoolean("experimental_tag_sync", false))
		CreateThread(NULL, 0, TagSyncUpdate, nullptr, 0, NULL);
}
#include "TagUpdate.h"
#include "../stdafx.h"
#include "../util/FileWatcher.h"
#include "../Common/H2EKCommon.h"
#include "../Common/tag_group_names.h"
#include "../util/string_util.h"
#include "../util/Logs.h"
#include <Shlobj.h>
#include <shlwapi.h>
#include <algorithm>
#include <set>

const static int millseconds_in_second = 1000;
const static float update_frequency = 1;
const static float max_valid_time = update_frequency * 5;

std::map<std::string, time_t> tags_being_saved;

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
			auto last_save = tags_being_saved.find(tolower(filename));
			if (last_save != tags_being_saved.end())
			{
				if (difftime(time(nullptr), last_save->second) <= max_valid_time) {
					pLog.WriteLog("Ignoring change to tag \"%s\" because it was modifed by us in the past %F seconds", filename, max_valid_time);
					return;
				} else {
					pLog.WriteLog("Ignoring being_saved state for \"%s\" as last update time is more than %F seconds ago ", filename, max_valid_time);
					tags_being_saved.erase(last_save);
				}
			}
			pLog.WriteLog("Reloading tag: \"%s\" : type: \"%s\"", tag_name.c_str(), file_ext.c_str());
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
		Sleep(static_cast<DWORD>(update_frequency * millseconds_in_second));

		for (auto it = tags_being_saved.begin(), ite = tags_being_saved.end(); it != ite;)
		{
			if (difftime(time(nullptr), it->second) > max_valid_time) {
				pLog.WriteLog("Unmarked tag \"%s\" as being_saved", it->first.c_str());
				it = tags_being_saved.erase(it);
			} else {
				++it;
			}
		}
	}
}

typedef char (__cdecl *TAG_SAVE)(int tag_index);
TAG_SAVE TAG_SAVE_ORG = reinterpret_cast<TAG_SAVE>(0x4B47C0);
char __cdecl TAG_SAVE_HOOK(int tag_index)
{
	typedef char *(__cdecl *Tag__GetName)(unsigned __int16 a1);
	Tag__GetName Tag__GetName_Impl = reinterpret_cast<Tag__GetName>(0x4AE940);
	typedef int (__cdecl *TAG_GET_GROUP_TAG)(unsigned __int16 a1);
	auto TAG_GET_GROUP_TAG_IMPL = reinterpret_cast<TAG_GET_GROUP_TAG>(0x004AE900);

	std::string tag_name = Tag__GetName_Impl(tag_index);
	int tag_group = TAG_GET_GROUP_TAG_IMPL(tag_index);

	std::string tag_file_name = tag_name + "." + tag_group_names.at(tag_group);

	tags_being_saved[tag_file_name] = time(nullptr);
	pLog.WriteLog("Marked tag \"%s\" as being_saved", tag_file_name.c_str());

	return TAG_SAVE_ORG(tag_index);
}


void H2SapienPatches::StartTagSync()
{
	if (conf.getBoolean("experimental_tag_sync", false)) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)TAG_SAVE_ORG, TAG_SAVE_HOOK);

		DetourTransactionCommit();
		CreateThread(NULL, 0, TagSyncUpdate, nullptr, 0, NULL);
	}
}
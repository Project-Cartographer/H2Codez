#include "TagUpdate.h"
#include "SapienInterface.h"
#include "util/FileWatcher.h"
#include "util/string_util.h"
#include "util/Logs.h"
#include "util/process.h"
#include "Common/BasicTagTypes.h"
#include "Common/H2EKCommon.h"
#include "Common/tag_group_names.h"
#include "Common/TagInterface.h"
#include "stdafx.h"
#include <shlwapi.h>
#include <algorithm>
#include <mutex>
#include <queue>

static struct tag_info {
	int group;
	std::string tag_name;
};

const static int millseconds_in_second = 1000;
const static float update_frequency = 1;
const static float max_valid_time = update_frequency * 5;
using namespace SapienInterface;
using namespace H2CommonPatches;

std::map<std::string, time_t> tags_being_saved;
std::queue<tag_info> tags_to_reload;
std::recursive_mutex tag_mutex;

class UpdateListener : public FW::FileWatchListener
{
public:
	UpdateListener() {}
	void handleFileAction(FW::WatchID watchid, const std::string& dir, const std::string& filename,
		FW::Action action)
	{
		std::cout << "DIR (" << dir + ") FILE (" + filename + ") has event " << action << std::endl;
		auto file_ext_pos = filename.find_last_of('.');
		if (action == FW::Action::Modified && file_ext_pos != string::npos
			&& file_ext_pos + 1 < filename.size() && get_global_structure_bsp() != NULL)
		{
			std::string file_ext = filename.substr(file_ext_pos + 1);
			std::string tag_name = filename.substr(0, file_ext_pos);

			auto tag_group = string_to_tag_group(file_ext);
			if (tag_group == NONE) // not a tag
				return;

			std::unique_lock<std::recursive_mutex> tag_lock(tag_mutex);
			auto last_save = tags_being_saved.find(tolower(filename));
			if (last_save != tags_being_saved.end())
			{
				if (difftime(time(nullptr), last_save->second) <= max_valid_time) {
					getLogger().WriteLog("Ignoring change to tag \"%s\" because it was modified by us in the past %F seconds", filename.c_str(), max_valid_time);
					return;
				}
				else {
					getLogger().WriteLog("Ignoring being_saved state for \"%s\" as last update time is more than %F seconds ago ", filename.c_str(), max_valid_time);
					tags_being_saved.erase(last_save);
				}
			}

			// hacky workaround for an event being issued twice
			if (tags_to_reload.empty() ||
					tags_to_reload.back().group != tag_group || tags_to_reload.back().tag_name != tag_name)
				tags_to_reload.push({ tag_group, tag_name });
			else
				getLogger().WriteLog("Not pushing tag for reloading (duplicated event)");
		}
	}
};

DWORD WINAPI TagSyncUpdate(
	_In_ LPVOID lpParameter
)
{
	UpdateListener listener;
	FW::FileWatcher fileWatcher;
	uint16_t watcher_count = 0;

	auto try_add_watcher = [&](const std::string &base_path) {
		try {
			fileWatcher.addWatch(base_path + "\\tags", &listener, true);
			watcher_count++;
		} catch (const std::exception &ex) {
			LOG_FUNC("exception: %s", ex.what());
		}
	};

	try_add_watcher(process::GetExeDirectoryNarrow());
	try_add_watcher(H2CommonPatches::get_h2ek_documents_dir());

	ASSERT_CHECK(watcher_count > 0); // if you have no tag folders you have a bigger issue

	while (true)
	{
		fileWatcher.update();
		Sleep(static_cast<DWORD>(update_frequency * millseconds_in_second));

		std::unique_lock<std::recursive_mutex> tag_lock(tag_mutex);
		for (auto it = tags_being_saved.begin(), ite = tags_being_saved.end(); it != ite;)
		{
			if (difftime(time(nullptr), it->second) > max_valid_time) {
				getLogger().WriteLog("Unmarked tag \"%s\" as being_saved", it->first.c_str());
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

	std::unique_lock<std::recursive_mutex> tag_lock(tag_mutex);
	tags_being_saved[tag_file_name] = time(nullptr);
	getLogger().WriteLog("Marked tag \"%s\" as being_saved", tag_file_name.c_str());
	tag_lock.unlock();

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

void H2SapienPatches::ProcessTagsToReload()
{
	std::unique_lock<std::recursive_mutex> tag_lock(tag_mutex);
	bool reload_sbsp = false;

	while (tags_to_reload.size() > 0)
	{
		auto tag_info = tags_to_reload.front();
		auto tag_ext = tag_group_names.at(tag_info.group).c_str();

		if (tags::is_tag_loaded(tag_info.group, tag_info.tag_name))
		{
			getLogger().WriteLog("Reloading tag: \"%s\" : type: \"%s\"", tag_info.tag_name.c_str(), tag_ext);
			switch (tag_info.group)
			{
			case 'sbsp':
			case 'ltmp':
			case 'scnr':
				reload_sbsp = true;
			default:
				tags::reload_tag(tag_info.group, tag_info.tag_name);
				break;
			}
		}
		else {
			getLogger().WriteLog("Ignoring change to tag: \"%s\" : type: \"%s\" because it's not loaded", tag_info.tag_name.c_str(), tag_ext);
		}
		tags_to_reload.pop();
	}
	if (reload_sbsp)
	{
		LOG_FUNC("Reloading sbsp");
		reload_structure_bsp();
	}
}

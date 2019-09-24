// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "Common\DiscordInterface.h"
#include "Common\H2EKCommon.h"
#include "Util\Debug.h"
#include "Tags\ShaderTag.h"
#include "util/string_util.h"
#include "H2Tool/CacheBuilder.h"

// use this to export a type to ida
struct __cache_header
{
	blam_tag magic;
	// Should be 8
	uint32_t engine_gen;

	uint32_t file_size;
	int field_C;

	uint32_t offset_to_index;
	uint32_t index_stream_size;
	uint32_t tag_buffer_size;
	uint32_t total_stream_size;
	uint32_t virtual_base_address;

	struct _tag_dependency_graph {
		uint32_t offset;
		uint32_t size;
	}tag_dependency_graph;

	long_string source_file;
	char version[32];
	enum scnr_type : int
	{
		SinglePlayer = 0,
		Multiplayer = 1,
		MainMenu = 2,
		MultiplayerShared = 3,
		SinglePlayerShared = 4
	};
	scnr_type type;
	crc32::result resource_crc;
	int shared_type;
	char field_158;
	char tracked__maybe;
	char field_15A;
	char field_15B;
	int field_15C;
	int field_160;
	int field_164;
	int field_168;

	struct _debug_string_id {
		uint32_t block_offset;
		uint32_t count;
		uint32_t buffer_size;
		uint32_t indices_offset;
		uint32_t buffer_offset;
	}string_ids;

	char dependency[4];
	FILETIME cache_build_dates[4];

	char name[32];
	int field_1C4;
	char tag_name[256];
	int minor_version;
	struct _debug_tag_names {
		uint32_t count;
		uint32_t buffer_offset;
		uint32_t buffer_size;
		uint32_t indices_offset;
	}tag_names;

	struct _language_pack {
		uint32_t offset;
		uint32_t size;
	}language_pack;

	int SecondarySoundGestaltDatumIndex;

	struct _fast_geometry_load_region {
		uint32_t cache_block_offset;
		uint32_t cache_block_size;
	}fast_geometry_load_region;

	int Checksum;
	int MoppCodesChecksum;
	BYTE field_2F8[1284];
	blam_tag foot;
};
__declspec(dllexport) __cache_header* block = nullptr;

#pragma region declarations

DWORD g_threadID;
HMODULE g_hModule;

H2EK_Globals game;

Logs pLog("H2Codez.log");
Settings conf("H2Codez.conf");

BOOL EnableDbgConsole;

#pragma endregion

__declspec(dllexport) void IIDking_import(void) {};

BOOL APIENTRY DllMain( HMODULE hModule,
	                   DWORD  ul_reason_for_call,
	                   LPVOID lpReserved
	                  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (__argc > 1)
		{
			// handle args
			for (int i = 1; i < __argc; i++)
			{
				std::string current_arg = __argv[i];

				// temp settings
				// -set:setting=value
				const static std::string arg_flag = "-set:";
				size_t pos = current_arg.find(arg_flag);
				if (pos != std::string::npos && pos + arg_flag.size() < current_arg.size())
				{
					std::string arg_contents = current_arg.substr(pos + arg_flag.size());
					size_t cut_point = arg_contents.find_first_of('=');
					if (cut_point == std::string::npos && cut_point + 1 < arg_contents.size())
						continue;
					std::string setting = arg_contents.substr(0, cut_point);
					std::string value = arg_contents.substr(cut_point + 1);

					str_trim(setting);
					str_trim(value);

					conf.setTempSetting(setting, value);
				}
			}
		}
		if (!conf.getBoolean("patches_enabled", true)) // enable basic stuff so launcher doesn't break
		{
			H2Toolz::minimal_init();
			break;
		}
		g_hModule = hModule;
		if (!H2Toolz::Init())
			std::exit(0);     
	    break;
	
	case DLL_PROCESS_DETACH:
		std::string cmd = GetCommandLineA();
		if (!g_process_crashed && game.process_type == H2EK::H2Tool 
				&& cmd.find("pause_after_run") != string::npos) {
			std::cout << "Press any key labeled enter to exit tool...." << std::endl;
			std::cin.get();
		}
		break;
	}
	return TRUE;
}

// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "Common\DiscordInterface.h"
#include "Common\H2EKCommon.h"
#include "Util\Debug.h"
#include "Tags\ShaderTag.h"
#include "util/string_util.h"

// use this to export a type to ida
//extern shader_block *block = new shader_block;

#pragma region declarations

DWORD g_threadID;
HMODULE g_hModule;

H2EK_Globals game;

Logs pLog("H2Codez.log");
Logs H2PCTool("H2PCTool.log");
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

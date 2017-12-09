#include "stdafx.h"
#include <codecvt>


//List of extra commands i found are contained here

static const s_tool_command_argument tool_build_structure_from_jms_arguments[] = {
	{
		_tool_command_argument_type_data_directory,
		L"jms file",
	"*.jms|*\\structure\\*.jms|"
	"Structure jms files must reside in a 'structure' sub-directory, for example, 'foo\\structure\\example.jms'",
	"JMS file you wish to create a structure BSP from",
	},
	{ _tool_command_argument_type_string,
	L"use release",
	NULL,
	"Name of Structure BSP",
	}
};
static const s_tool_command tool_build_structure_from_jms = {
	L"structure new from jms",
	CAST_PTR(_tool_command_proc, 0x420220),
	tool_build_structure_from_jms_arguments,	NUMBEROF(tool_build_structure_from_jms_arguments),
	false
};

static const char* get_h2tool_version()
{
	typedef char*(_cdecl* _get_h2tool_build_date)();
	static _get_h2tool_build_date get_h2tool_build_date = CAST_PTR(_get_h2tool_build_date, 0xEA760);

	return get_h2tool_build_date();

}
static int __cdecl TAG_LOAD(int tag_type, cstring tags_directory, int a3)
{
	typedef int(_cdecl* _TAG_LOAD)(int,cstring,int);
	static _TAG_LOAD TAG_LOAD_ = CAST_PTR(_TAG_LOAD, 0x533930);

	return TAG_LOAD_(tag_type,tags_directory,a3);
}

#pragma region Notes on H2ToolDev_commands

//Let me Some up whats H2Tool Dev commands
//While researching through h2tool i found some lost functions one after another like a table which seemed to be not in use anymore
//u can go to this offset TABLE_START = 0x97A910;
//There u will find these functions probably around 20 with some of them nulled out to do nothing
//I myself couldn't understand how to use those functions after writing the codes to call them cuz i couldn't make out what do those take as a parameter
//I can just assmume they were development functions used by Bungie or Hired Gun to perform Tag conversions(HCE->H2) or make their work easier
//If u ever figure out how they work,Plz tell me too ///RIP THIS LINE
//Finally Sorted out :)
#pragma endregion 

struct dev_command {
	cstring command_name;
	cstring command_description;
	DWORD tag_type; // or char[4] in Little-endian
	DWORD unk_1;
	DWORD unk_2;
	DWORD unk_3;
	char(__cdecl *command_impl)(wchar_t*, int);

};
static_assert(sizeof(dev_command) == 0x1C, "Invalid struct size for dev_command");

static dev_command *GetDevCommandByName(wcstring W_function_name)
{
	std::string function_name = wstring_to_string.to_bytes(W_function_name);
	dev_command *command_table = reinterpret_cast<dev_command*>(0x97A910);
	for (int i = 0; i <= 0x43; i++) {
		dev_command *current_cmd = (command_table + i);
		if (function_name == current_cmd->command_name)
			return current_cmd;
	}
	return nullptr;
}

void _cdecl list_all_extra_commands_proc(wcstring* arguments)
{
	printf("\n");
	dev_command *command_table = reinterpret_cast<dev_command*>(0x97A910);
	for (int i = 0; i <= 0x43; i++) {
		dev_command *current_cmd = (command_table + i);
		printf("  %s : %s\n", current_cmd->command_name, current_cmd->command_description);
		H2PCTool.WriteLog(current_cmd->command_name);//Store It in log
	}
	return;
}

static void _cdecl h2dev_extra_commands_proc(wcstring* arguments)
{
	wcstring command_name = arguments[0];
	wcstring command_parameter_0 = arguments[1];
	if (wcscmp(command_name, L"list") == 0) {
		list_all_extra_commands_proc(nullptr);
	}
	else if (wcscmp(command_name, L"help") == 0) {

		if (command_parameter_0)
		{
			dev_command *cmd = GetDevCommandByName(command_parameter_0);
			if (!cmd)
			{
				printf("\n  Wrong <command_name>");
				printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");
				return;
			}
			printf("\n  usage : %s\n  Description : %s\n", cmd->command_name, cmd->command_description);
			return;
		}
		else
			printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");



	}
	//Dev command usage block
	else
	{
		dev_command *cmd = GetDevCommandByName(command_name);
		if (!cmd)
		{
			printf("\n  Wrong <command_name>");
			printf("\n  use : extra-commands help <command_name>");
			return;
		}
		else
		{
			std::string f_parameter = wstring_to_string.to_bytes(command_parameter_0);
			H2PCTool.WriteLog("Tag Type %X \n %s", cmd->tag_type, f_parameter);
			DWORD tag_index = TAG_LOAD(cmd->tag_type, f_parameter.c_str(), 7);

			if (cmd->command_impl(nullptr, tag_index))// call Function via address			
				return;
			printf("\n  usage : %s\n  Description : %s\n", cmd->command_name, cmd->command_description);
		}

		printf("\n  No such command present.");
		return;
	}
}

static const s_tool_command_argument h2dev_extra_commands_arguments[] = {
	{
		_tool_command_argument_type_string,
		L"command name",
	},
	{
		_tool_command_argument_type_string,
		L"command arguments",
	}
};

static const s_tool_command h2dev_extra_commands_defination = {
	L"extra commands",
	CAST_PTR(_tool_command_proc,h2dev_extra_commands_proc),
	h2dev_extra_commands_arguments,	NUMBEROF(h2dev_extra_commands_arguments),
	false
};

static const s_tool_command list_extra_commands = {
	L"extra commands list",
	CAST_PTR(_tool_command_proc,list_all_extra_commands_proc),
	nullptr, 0,
	false
};


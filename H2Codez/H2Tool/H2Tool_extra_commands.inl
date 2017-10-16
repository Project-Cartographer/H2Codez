#include "stdafx.h"


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

static DWORD GetH2Tool_Dev__by_name(wcstring function_name)
{
	char buffer[_MAX_PATH];
	wstring_to_string(buffer, sizeof(buffer), function_name, -1);
	int TABLE_START = 0x97A910;
	int TABLE_END = 0x97B064;
	for (;TABLE_START <= TABLE_END;TABLE_START += 0x1C)
	{
		cstring command_name = CAST_PTR(cstring, *(DWORD*)TABLE_START);
		if (strcmp(buffer, command_name) == 0)
			return TABLE_START;


	}
	return 0;
}


static void _cdecl h2dev_extra_commands_proc(wcstring* arguments)
{

	wcstring command_name = arguments[0];
	wcstring command_parameter_0 = arguments[1];
	if (wcscmp(command_name, L"list") == 0)
	{

		if (wcscmp(command_parameter_0, L"all") == 0)
		{
			printf("\n");
			int TABLE_START = 0x97A910;
			int TABLE_END = 0x97B064;
			for (;TABLE_START <= TABLE_END;TABLE_START += 0x1C)
			{
				cstring name = CAST_PTR(cstring, *(DWORD*)TABLE_START);
				printf("  %s\n", name);
				H2PCTool.WriteLog(name);//Store It in log
			}
			return;
		}
		else if (stoll(command_parameter_0) > 0)
		{
			printf("\n");
			int TABLE_START = 0x97A910;
			int TABLE_END = 0x97B064;
			for (int a = 1;TABLE_START <= TABLE_END;TABLE_START += 0x1C, a++)
			{
				cstring name = CAST_PTR(cstring, *(DWORD*)TABLE_START);
				printf("  %s\n", name);
				H2PCTool.WriteLog(name);//Store It log
				if (a == stoll(command_parameter_0))
					return;
			}
			return;
		}
		else
			printf("\n  usage : extra-commands list <count,all>\n  Description : all -> Prints all the extra-commands <command_name>.\n  Description: count->Prints a Range of the extra-commands <command_name> ");
	}

	else if (wcscmp(command_name, L"help") == 0)
	{

		if (command_parameter_0)
		{
			int TABLE_START = GetH2Tool_Dev__by_name(command_parameter_0);
			if (!TABLE_START)
			{
				printf("\n  Wrong <command_name>");
				printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");
				return;
			}
			cstring name = CAST_PTR(cstring, *(DWORD*)TABLE_START);
			cstring description = CAST_PTR(cstring, *(DWORD*)(TABLE_START + 4));
			printf("\n  usage : %s\n  Description : %s\n", name, description);
			return;
		}
		else
			printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");



	}
	//Dev command usage block
	else
	{
		int TABLE_START = GetH2Tool_Dev__by_name(command_name);
		if (!TABLE_START)
		{
			printf("\n  Wrong <command_name>");
			printf("\n  use : extra-commands help <command_name>");
			return;
		}
		else
		{

			cstring f_name = CAST_PTR(cstring, *(DWORD*)TABLE_START);
			DWORD f_tag_type =  *(DWORD*)(TABLE_START + 8);

			char f_parameter[MAX_PATH];		
			wstring_to_string(f_parameter, sizeof(f_parameter), command_parameter_0, -1);
			H2PCTool.WriteLog("Tag Type %X \n %s",f_tag_type,f_parameter);			
			DWORD tag_index = TAG_LOAD(f_tag_type, f_parameter, 7);

			if(((char(__cdecl *)( wchar_t* ,int))*(DWORD*)(TABLE_START+0x18))(0, tag_index))// call Function via address			
			return;
			cstring f_description = CAST_PTR(cstring, *(DWORD*)(TABLE_START + 4));
			printf("\n  usage : %s\n  Description : %s\n", f_name, f_description);
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




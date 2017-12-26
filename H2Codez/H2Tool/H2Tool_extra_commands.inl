#include "stdafx.h"
#include "H2ToolLibrary.inl"
#include "H2Tool_Render_Model.h"
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
bool _cdecl tool_build_structure_from_jms_proc(wcstring* args)
{
	typedef bool(_cdecl* _tool_build_structure_from_jms_proc)(wcstring*);
	static _tool_build_structure_from_jms_proc tool_build_structure_from_jms_proc_ = CAST_PTR(_tool_build_structure_from_jms_proc, 0x420220);
	return tool_build_structure_from_jms_proc_(args);

}
static const s_tool_command tool_build_structure_from_jms = {
	L"structure new from jms",
	CAST_PTR(_tool_command_proc, tool_build_structure_from_jms_proc),
	tool_build_structure_from_jms_arguments,	NUMBEROF(tool_build_structure_from_jms_arguments),
	false
};

#pragma region H2ToolDev_commands
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

static DWORD GetH2Tool_Dev__by_name(wcstring W_function_name)
{
	std::string function_name = wstring_to_string.to_bytes(W_function_name);
	int TABLE_START = 0x97A910;
	int TABLE_END = 0x97B064;
	for (;TABLE_START <= TABLE_END;TABLE_START += 0x1C)
	{
		s_tool_h2dev_command *cmd = new (CAST_PTR(void*, TABLE_START))s_tool_h2dev_command;
		if (strcmp(function_name.c_str(), cmd->name) == 0)
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
				s_tool_h2dev_command *cmd = new (CAST_PTR(void*, TABLE_START))s_tool_h2dev_command;
				printf("  %s\n", cmd->name);
				H2PCTool.WriteLog(cmd->name);//Store It in log
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
				s_tool_h2dev_command *cmd = new (CAST_PTR(void*, TABLE_START))s_tool_h2dev_command;
				printf("  %s\n", cmd->name);
				H2PCTool.WriteLog(cmd->name);//Store It log
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
			s_tool_h2dev_command *cmd = new (CAST_PTR(void*, TABLE_START))s_tool_h2dev_command;
			printf("\n  usage : %s\n  Description : %s\n", cmd->name, cmd->description);
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
			s_tool_h2dev_command *cmd = new (CAST_PTR(void*, TABLE_START))s_tool_h2dev_command;
			std::string f_parameter = wstring_to_string.to_bytes(command_parameter_0);
			DWORD tag_index = TAG_LOAD(cmd->tag_type, f_parameter.c_str(), 7);
			if (tag_index != -1)				
				if (cmd->import_proc(0, tag_index))
					return;
			printf("\n  Unable to Load tag\n");
			printf("\n  usage : %s\n  Description : %s\n", cmd->name, cmd->description);
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

#pragma endregion
#pragma region Render_model_import

bool k_render_model_imported = FALSE;
static WCHAR out_path[256];
static char c_out_path[256];
static cstring render_model_folder = "render";
std::string target_folder;// a String that holds the containing_folder of the current generated tags
static int global_geometry_imported_count = 0;
tag_data_struct** global_sbsp_data_list;

static void _cdecl TAG_RENDER_MODEL_IMPORT_PROC(s_file_reference& sFILE_REF, char* _TAG_INDEX_)
{
	DWORD TAG_INDEX = (DWORD)_TAG_INDEX_;
	DWORD MODE_TAG = TAG_GET('mode', TAG_INDEX);
	DWORD import_info_block_offset = MODE_TAG + 0xC;

	if (MODE_TAG != -1) {
		if (!TAG_ADD_IMPORT_INFO_ADD_DATA_(CAST_PTR(void*, import_info_block_offset), sFILE_REF))
		{
			k_render_model_imported = FALSE;
			return;
		}

		printf("    == Import info  Added \n");
		
		char jms_file_path[256];
		GetFileAttributefromFILE(sFILE_REF, ATTRIBUTES_TYPE::IMPORT_FILE_FULL_PATH, jms_file_path);

		WCHAR w_path[256];
		MultiByteToWideChar(0xFDE9u, 0, jms_file_path, 0xFFFFFFFF, w_path, 0x104);

		//generating sbsp from jms
		wcstring p[2] = { w_path,L"sbsp_temp" };
		if (!tool_build_structure_from_jms_proc(p))		
			return;
		

		char sbsp_file_name[256];
		GetFileAttributefromFILE(sFILE_REF, ATTRIBUTES_TYPE::FILE_NAME, sbsp_file_name);

		std::string sbsp_file = target_folder;
		sbsp_file.append("\\");
		sbsp_file.append(sbsp_file_name);	
		sbsp_file.append(".scenario_structure_bsp");	

		
		ifstream fin;
		fin.open(sbsp_file.c_str(), ios::binary | ios::in | ios::ate);
		DWORD sbsp_size = fin.tellg();
		fin.seekg(0x0, ios::beg);
		

		char* sbsp_data = new char[sbsp_size];
		fin.read(sbsp_data, sbsp_size);

		fin.close();
		
		DeleteFile(sbsp_file.c_str());
		printf("    == deleted %s.scenario_structure_bsp  \n",sbsp_file_name);

		if (global_geometry_imported_count == 0)
		{
			//haven't intialised
			global_sbsp_data_list = new tag_data_struct*[1];
			
			tag_data_struct* temp = new tag_data_struct();
			temp->tag_data = sbsp_data;
			temp->size = sbsp_size;

			global_sbsp_data_list[0] = temp;
		}
		else
		{
			//have intialised
			tag_data_struct** temp = new tag_data_struct*[global_geometry_imported_count + 1];

			//copy the stuff
			for (int i = 0; i < global_geometry_imported_count; i++)
				temp[i] = global_sbsp_data_list[i];

			tag_data_struct* tempy = new tag_data_struct();
			tempy->tag_data = sbsp_data;
			tempy->size = sbsp_size;

			temp[global_geometry_imported_count] = tempy;

			delete[] global_sbsp_data_list;
			global_sbsp_data_list = temp;
		}

		global_geometry_imported_count++;	

		printf("    == leaving TAG_RENDER_MODEL_IMPORT_PROC\n");

		k_render_model_imported = TRUE;
		return;


	}
	k_render_model_imported = FALSE;
	return;

}
static const s_tool_import_definations_ TAG_RENDER_IMPORT_DEFINATIONS_[] = {
	"jms",
	CAST_PTR(_tool_import__defination_proc,TAG_RENDER_MODEL_IMPORT_PROC),
	0,
	0,
};

static void *jms_collision_geometry_import_defination_ = CAST_PTR(void*, 0x97C350);
static bool _cdecl h2pc_generate_render_model_(DWORD TAG_INDEX, s_file_reference& FILE_REF)
{
	
	DWORD mode_tag_file = TAG_GET('mode', TAG_INDEX);
	DWORD import_info_block_offset = mode_tag_file + 0xC;

	DWORD SBSP_FOLDER_LOAD_1 = 0x41C835;
	DWORD SBSP_FOLDER_LOAD_2 = 0x41F52D;

	//replacing 'structure' folder text with 'render' folder
	WritePointer((DWORD)(SBSP_FOLDER_LOAD_1), (void*)render_model_folder);
	WritePointer((DWORD)(SBSP_FOLDER_LOAD_2), (void*)render_model_folder);

	
	WideCharToMultiByte(0xFDE9u, 0, out_path, 0xFFFFFFFF, c_out_path, 0x100, 0, 0);
	target_folder = app_directory;
	target_folder.append("\\tags\\");
	target_folder.append(c_out_path);

	if (load_model_object_definations_(import_info_block_offset, jms_collision_geometry_import_defination_, 1, FILE_REF))
	{
		if (TAG_ADD_IMPORT_INFO_BLOCK(CAST_PTR(void*, import_info_block_offset)))
		{
			int defination_addr = (int)&TAG_RENDER_IMPORT_DEFINATIONS_;
			use_import_definitions(CAST_PTR(void*, defination_addr), 1, FILE_REF, (void*)TAG_INDEX, 0);
			if (k_render_model_imported && global_geometry_imported_count>0)
			{
				printf("    == saving temporary render_model  \n");
				TAG_SAVE(TAG_INDEX);//creating the current render_model file in Disk
				TAG_UNLOAD(TAG_INDEX);


				std::string render_model_file_name_ = strrchr(c_out_path, '\\');
				render_model_file_name_ = render_model_file_name_.substr(1).c_str();

				std::string mode_file = target_folder;
				mode_file.append("\\");
				mode_file.append(render_model_file_name_);

				std::string scnr_file = mode_file.c_str();
				scnr_file.append(".scenario");
				mode_file.append(".render_model");

				DeleteFile(scnr_file.c_str());
				printf("    == deleted %s.scenario  \n", render_model_file_name_.c_str());

				ifstream fin;
				fin.open(mode_file.c_str(), ios::binary | ios::in | ios::ate);
				DWORD mode_size = fin.tellg();
				fin.seekg(0x0, ios::beg);

				char* mode_data = new char[mode_size];
				fin.read(mode_data, mode_size);

				fin.close();
				printf("    == generating new %s.render_model  \n", render_model_file_name_.c_str());

				sbsp_mode* obj = new sbsp_mode(mode_data, mode_size);
				obj->Add_sbps_DATA(global_sbsp_data_list, global_geometry_imported_count);

				tag_data_struct* lol = obj->Get_Tag_DATA();

				ofstream fout;
				fout.open(mode_file.c_str(), ios::binary | ios::out);
				fout.write(lol->tag_data, lol->size);
				fout.close();

				printf("    == Added Cluster Data  \n");
				printf("      ### saved render model file '%s' ", render_model_file_name_.c_str());

			}

		}
	}
	return k_render_model_imported;

}
static bool _cdecl h2pc_import_render_model_proc(wcstring* arguments)
{

	s_file_reference reference;

	WCHAR WideCharStr[256];
	char MultiByteStr[256];
	bool b_render_imported = true;


	if (tool_build_paths(arguments[0], "render", reference, out_path, WideCharStr))
	{
		WideCharToMultiByte(0xFDE9u, 0, WideCharStr, 0xFFFFFFFF, MultiByteStr, 0x100, 0, 0);
		DWORD TAG_INDEX = TAG_LOAD('mode', MultiByteStr, 7);
		if (TAG_INDEX != -1)
		{

			DWORD RENDER_MODEL_TAG = TAG_GET('mode', TAG_INDEX);
			DWORD import_info_field = RENDER_MODEL_TAG + 0xC;

			if (!load_model_object_definations_(import_info_field, jms_collision_geometry_import_defination_, 1, reference))
				b_render_imported = false;

			TAG_UNLOAD(TAG_INDEX);
			if (!b_render_imported)
				return b_render_imported;
		}
		char render_import_file_name[256]; GetFileAttributefromFILE(reference, ATTRIBUTES_TYPE::CONTAINING_FOLDER, render_import_file_name);
		printf("        ### creating new render model file with name '%s' \n ", render_import_file_name);
		TAG_INDEX = TAG_NEW('mode', MultiByteStr);

		if (TAG_INDEX != -1)
		{
			if (TAG_FILE_CHECK_READ_ONLY_ACCESS(TAG_INDEX, 0))
			{
				if (h2pc_generate_render_model_(TAG_INDEX, reference))
				{
					b_render_imported = true;
					//TAG_SAVE(TAG_INDEX);				

				}
				else
				{
					printf("      ### FATAL ERROR unable to generate render model '%s' \n", render_import_file_name);
					b_render_imported = false;
				}
			}
			else
			{
				printf("      ### ERROR render model '%s' is not writable\n", render_import_file_name);
				TAG_UNLOAD(TAG_INDEX);
				b_render_imported = false;
			}
		}
		else
		{
			printf("     ### ERROR unable to create render model '%s'\n", render_import_file_name);
			b_render_imported = false;
		}

	}
	else
	{
		wprintf(L"### ERROR unable to find 'render' data directory for '%s' ", arguments[0]);
		b_render_imported = false;
	}
	return b_render_imported;
}

#pragma endregion

#include "stdafx.h"
#include "H2tool.h"
#include "H2ToolLibrary.inl"
#include "H2Tool_Render_Model.h"
#include "LightMapping.h"
#include "Common/FiloInterface.h"
#include "Common/TagInterface.h"
#include "Common/Pathfinding.h"
#include "Common/tag_group_names.h"
#include "Common/TagDumper.h"
#include "Common/H2EKCommon.h"
#include "util/string_util.h"
#include "util/time.h"
#include "Tags/ScenarioStructureBSP.h"
#include "Tags/ScenarioStructureLightmap.h"
#include "Tags/ScenarioTag.h"
#include "Tags/Bitmap.h"
#include "util/Patches.h"
#include "util/process.h"
#include <iostream>
#include <sstream>
#include <codecvt>
#include <unordered_set>
#include <direct.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#define extra_commands_count 0x43
#define help_desc "Prints information about the command name passed to it"
#define list_all_desc "lists all extra commands"

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
bool _cdecl tool_build_structure_from_ass_proc(wcstring* args)
{
	typedef bool(_cdecl* _tool_build_structure_from_ass_proc)(wcstring*);
	static _tool_build_structure_from_ass_proc tool_build_structure_from_ass_proc_ = CAST_PTR(_tool_build_structure_from_ass_proc, 0x4201E0);
	return tool_build_structure_from_ass_proc_(args);

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

static s_tool_h2dev_command *GetDevCommandByName(wcstring W_function_name)
{
	std::string function_name = wstring_to_string.to_bytes(W_function_name);
	s_tool_h2dev_command *command_table = reinterpret_cast<s_tool_h2dev_command*>(0x97A910);
	for (int i = 0; i <= extra_commands_count; i++) {
		s_tool_h2dev_command *current_cmd = (command_table + i);
		if (function_name == current_cmd->command_name)
			return current_cmd;
	}
	return nullptr;
}

void _cdecl list_all_extra_commands_proc(wcstring* arguments)
{
	s_tool_h2dev_command *command_table = reinterpret_cast<s_tool_h2dev_command*>(0x97A910);
	printf("\n  help : " help_desc);
	printf("\n  list all : " list_all_desc);
	for (int i = 0; i <= extra_commands_count; i++) {
		s_tool_h2dev_command *current_cmd = (command_table + i);
		printf("\n  %s : %s", current_cmd->command_name, current_cmd->command_description);
		H2PCTool.WriteLog(current_cmd->command_name);//Store It in log
	}
}

static void _cdecl h2dev_extra_commands_proc(wchar_t ** arguments)
{
	wchar_t *command_name = arguments[0];
	wchar_t *command_parameter_0 = arguments[1];
	_wcslwr(command_name);
	_wcslwr(command_parameter_0);

	if (wcscmp(command_name, L"list") == 0) {
		list_all_extra_commands_proc(nullptr);
	}
	else if (wcscmp(command_name, L"help") == 0) {
		if (command_parameter_0) {
			s_tool_h2dev_command *cmd = GetDevCommandByName(command_parameter_0);
			if (!cmd) {
				if (wcscmp(command_parameter_0, L"help") == 0) {
					printf("\n  usage : help\n  Description : " help_desc);
					return;
				}
				if (wcscmp(command_parameter_0, L"list") == 0) {
					printf("\n  usage : list all\n  Description : " list_all_desc);
					return;
				}
				printf("\n  Wrong <command_name>");
				printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");
				return;
			}
			printf("\n  usage : %s\n  Description : %s\n", cmd->command_name, cmd->command_description);
			return;
		}
		else {
			printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");
		}



	}
	//Dev command usage block
	else
	{
		s_tool_h2dev_command *cmd = GetDevCommandByName(command_name);
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
			datum tag = tags::load_tag(cmd->tag_type, f_parameter.c_str(), 7);

			if (cmd->command_impl(nullptr, tag))// call Function via address			
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

#pragma endregion
#pragma region Render_model_import

bool k_render_model_imported = FALSE;
static WCHAR out_path[256];
static char c_out_path[256];
static cstring render_model_folder = "render";
std::string target_folder;// a String that holds the containing_folder of the current generated tags
static int global_geometry_imported_count = 0;
tag_data_struct** global_sbsp_data_list;

static void _cdecl TAG_RENDER_MODEL_IMPORT_PROC(file_reference *sFILE_REF, char* _TAG_INDEX_)
{
	DWORD TAG_INDEX = (DWORD)_TAG_INDEX_;
	DWORD MODE_TAG = (DWORD)tags::get_tag('mode', TAG_INDEX);
	DWORD import_info_block_offset = MODE_TAG + 0xC;

	if (MODE_TAG != -1) {
		if (!TAG_ADD_IMPORT_INFO_ADD_DATA_(CAST_PTR(void*, import_info_block_offset), sFILE_REF))
		{
			k_render_model_imported = FALSE;
			return;
		}

		printf("    == Import info  Added \n");

		std::string path = FiloInterface::get_path_info(sFILE_REF, PATH_FLAGS::FULL_PATH);
		std::string file_type= FiloInterface::get_path_info(sFILE_REF, PATH_FLAGS::FILE_EXTENSION);

		WCHAR w_path[256];
		MultiByteToWideChar(0xFDE9u, 0, path.c_str(), 0xFFFFFFFF, w_path, 0x104);

		//generating sbsp from jms
		wcstring p[2] = { w_path,L"sbsp_temp" };
		
		if(strcmp(file_type.c_str(),"jms")==0)
			tool_build_structure_from_jms_proc(p);
		else
			tool_build_structure_from_ass_proc(p);	
		

		std::string sbsp_file = target_folder + "\\" + FiloInterface::get_path_info(sFILE_REF, PATH_FLAGS::FILE_NAME) + ".scenario_structure_bsp";

		ifstream fin;
		fin.open(sbsp_file.c_str(), ios::binary | ios::in | ios::ate);
		// sbsp isn't going to be larger than 4 gigs
		DWORD sbsp_size = static_cast<size_t>(fin.tellg());
		fin.seekg(0x0, ios::beg);

		if (sbsp_size == 0)
		{
			printf("    == failed to import geometry \n   ===SKIPPPING %s \n", FiloInterface::get_path_info(sFILE_REF, PATH_FLAGS::FILE_NAME).c_str());
			fin.close();
			return;
		}

		char* sbsp_data = new char[sbsp_size];
		fin.read(sbsp_data, sbsp_size);

		fin.close();

		//		DeleteFile(sbsp_file.c_str());
		//		printf("    == deleted %s.scenario_structure_bsp  \n",sbsp_file_name);

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
	{
	"jms",
	CAST_PTR(_tool_import__defination_proc,TAG_RENDER_MODEL_IMPORT_PROC),
	0,
	0,
	},

	{
	"ass",
	CAST_PTR(_tool_import__defination_proc,TAG_RENDER_MODEL_IMPORT_PROC),
	0,
	0,
	}
	
};

static void *jms_collision_geometry_import_defination_ = CAST_PTR(void*, 0x97C350);
static bool _cdecl h2pc_generate_render_model_(DWORD TAG_INDEX, file_reference& FILE_REF)
{

	DWORD mode_tag_file = (DWORD)tags::get_tag('mode', TAG_INDEX);
	DWORD import_info_block_offset = mode_tag_file + 0xC;

	DWORD SBSP_FOLDER_LOAD_1 = 0x41C835;
	DWORD SBSP_FOLDER_LOAD_2 = 0x41F52D;

	//replacing 'structure' folder text with 'render' folder
	WritePointer(SBSP_FOLDER_LOAD_1, render_model_folder);
	WritePointer(SBSP_FOLDER_LOAD_2, render_model_folder);


	WideCharToMultiByte(0xFDE9u, 0, out_path, 0xFFFFFFFF, c_out_path, 0x100, 0, 0);
	target_folder = app_directory;
	target_folder.append("\\tags\\");
	target_folder.append(c_out_path);

	if (load_model_object_definations_(import_info_block_offset, jms_collision_geometry_import_defination_, 1, FILE_REF))
	{
		if (TAG_ADD_IMPORT_INFO_BLOCK(CAST_PTR(void*, import_info_block_offset)))
		{
			int defination_addr = (int)&TAG_RENDER_IMPORT_DEFINATIONS_;
			use_import_definitions(CAST_PTR(void*, defination_addr), 2, FILE_REF, (void*)TAG_INDEX, 0);
			if (k_render_model_imported && global_geometry_imported_count > 0)
			{
				printf("    == saving temporary render_model  \n");
				tags::save_tag(TAG_INDEX);//creating the current render_model file in Disk
				tags::unload_tag(TAG_INDEX);


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
				DWORD mode_size = static_cast<size_t>(fin.tellg());
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

	file_reference file_reference;

	WCHAR wide_path[256];
	std::string path;
	bool b_render_imported = true;


	if (tool_build_paths(arguments[0], "render", file_reference, out_path, &wide_path))
	{
		path = wstring_to_string.to_bytes(wide_path);
		datum TAG_INDEX = tags::load_tag('mode', path.c_str(), 7);
		if (TAG_INDEX.is_valid())
		{
			DWORD RENDER_MODEL_TAG = (DWORD)tags::get_tag('mode', TAG_INDEX);
			DWORD import_info_field = RENDER_MODEL_TAG + 0xC;

			if (!load_model_object_definations_(import_info_field, jms_collision_geometry_import_defination_, 1, file_reference))
				b_render_imported = false;

			tags::unload_tag(TAG_INDEX);
			if (!b_render_imported)
				return b_render_imported;
		}
		auto dir_name = FiloInterface::get_path_info(&file_reference, PATH_FLAGS::CONTAINING_DIRECTORY_NAME);
		printf("        ### creating new render model file with name '%s' \n ", dir_name.c_str());
		TAG_INDEX = tags::new_tag('mode', path);

		if (TAG_INDEX.is_valid())
		{
			if (TAG_FILE_CHECK_READ_ONLY_ACCESS(TAG_INDEX.as_long(), false))
			{
				if (h2pc_generate_render_model_(TAG_INDEX.as_long(), file_reference))
				{
					b_render_imported = true;
					//TAG_SAVE(TAG_INDEX);				

				}
				else
				{
					printf("      ### FATAL ERROR unable to generate render model '%s' \n", dir_name.c_str());
					b_render_imported = false;
				}
			}
			else
			{
				printf("      ### ERROR render model '%s' is not writable\n", dir_name.c_str());
				tags::unload_tag(TAG_INDEX);
				b_render_imported = false;
			}
		}
		else
		{
			printf("     ### ERROR unable to create render model '%s'\n", dir_name.c_str());
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

static const s_tool_command list_extra_commands = {
	L"extra commands list",
	CAST_PTR(_tool_command_proc,list_all_extra_commands_proc),
	nullptr, 0,
	true
};

std::string filesystem_path_to_tag_path(const wchar_t *fs_path, blam_tag *tag_type = nullptr)
{
	std::string path = tolower(wstring_to_string.to_bytes(fs_path));
	file_info info = get_file_path_info(path);

	if (tag_type)
	{
		if (info.has_entension)
			*tag_type = H2CommonPatches::string_to_tag_group(info.extension);
		else
			*tag_type = NONE;
	}

	return info.file_path;
}

/*
	Check if a sbsp DOESN'T have pathfinding data and prompt user otherwise
*/
bool check_pathfinding_clear(scenario_structure_bsp_block *target)
{
	if (target->pathfindingData.size > 0)
	{
		std::cout << "bsp already has pathfinding. Do you want to overwrite it? (Y/N)" << std::endl;
		while (std::cin)
		{
			std::string input;
			std::cin >> input;
			str_trim(input);
			input = tolower(input);
			if (input.size() >= 1)
			{
				if (input[0] == 'y')
				{
					std::cout << "Clearing old pathfinding data" << std::endl;
					tags::block_delete_all(&target->pathfindingData);
					return true;
				}
				else if (input[0] == 'n')
				{
					return false;
				}
			}
		}
		return false;
	}
	return true;
}

void _cdecl pathfinding_from_coll_proc(const wchar_t *argv[])
{
	auto load_tag = [](const std::string &path, bool can_save) -> datum
	{
		datum tag = tags::load_tag('sbsp', path.c_str(), can_save ? 1 : 7);
		if (tag.index == NONE)
		{
			printf_s("Failed to load tag '%s', aborting\n", path.c_str());
		}
		return tag;
	};

	std::string sbsp_path = filesystem_path_to_tag_path(argv[0]);
	printf_s("sbsp :'%s'\n", sbsp_path.c_str());

	datum sbsp = load_tag(sbsp_path, false);

	if (sbsp.index == NONE)
		return;
	scenario_structure_bsp_block *sbsp_data = tags::get_tag<scenario_structure_bsp_block>('sbsp', sbsp);
	if (!check_pathfinding_clear(sbsp_data))
		return;

	pathfinding::generate(sbsp);

	if (!tags::save_tag(sbsp))
		printf_s("Failed to save tag!\n");

	tags::unload_tag(sbsp);
	printf_s("Done!");
}

const s_tool_command_argument pathfinding_from_coll_args[] =
{
	{ _tool_command_argument_type_tag_name, L"sbsp", ".scenario_structure_bsp" },
};

static const s_tool_command pathfinding_from_coll
{
	L"pathfinding from coll",
	pathfinding_from_coll_proc,
	pathfinding_from_coll_args,
	ARRAYSIZE(pathfinding_from_coll_args),
	false
};

const s_tool_command_argument lightmap_slave_args[] =
{
	{ _tool_command_argument_type_tag_name, L"scenario", "*.scenario" },
	{ _tool_command_argument_type_string, L"bsp name" },
	{ _tool_command_argument_type_radio, L"quality setting", "checkerboard|draft_low|draft_medium|draft_high|draft_super|direct_only|low|medium|high|super" },
	{ _tool_command_argument_type_0, L"slave count" },
	{ _tool_command_argument_type_0, L"slave index" }
};

const s_tool_command_argument lightmap_master_args[] =
{
	{ _tool_command_argument_type_tag_name, L"scenario", "*.scenario" },
	{ _tool_command_argument_type_string, L"bsp name" },
	{ _tool_command_argument_type_radio, L"quality setting", "checkerboard|draft_low|draft_medium|draft_high|draft_super|direct_only|low|medium|high|super" },
	{ _tool_command_argument_type_0, L"slave count" }
};

static const s_tool_command lightmaps_slave
{
	L"lightmaps slave",
	generate_lightmaps_slave,
	lightmap_slave_args,
	ARRAYSIZE(lightmap_slave_args),
	true
};

static const s_tool_command lightmaps_slave_fork
{
	L"lightmaps slave fork",
	generate_lightmaps_fork_slave,
	lightmap_master_args,
	ARRAYSIZE(lightmap_master_args),
	true
};

static const s_tool_command lightmaps_master
{
	L"lightmaps master",
	generate_lightmaps_master,
	lightmap_master_args,
	ARRAYSIZE(lightmap_master_args),
	true
};

static const s_tool_command lightmaps_local_mp
{
	L"lightmaps local multi process",
	generate_lightmaps_local_multi_process,
	lightmap_master_args,
	ARRAYSIZE(lightmap_master_args),
	true
};

void _cdecl fix_extracted_lightmaps(const wchar_t *argv[])
{
	std::string scnr_path = filesystem_path_to_tag_path(argv[0]);
	printf_s("scnr :'%s'\n", scnr_path.c_str());

	datum scenario = tags::load_tag('scnr', scnr_path.c_str(), 7);

	if (!scenario.is_valid())
	{
		printf("Unable to load tag %s\n", scnr_path.c_str());
		return;
	}

	auto scenario_data = tags::get_tag<scnr_tag>('scnr', scenario);
	for (const auto &bsp_ref : scenario_data->structureBSPs)
	{
		printf(" == bsp: %s ==\n", bsp_ref.structureBSP.tag_name);
		datum bsp_tag = tags::load_tag('sbsp', bsp_ref.structureBSP.tag_name, 7);
		datum lightmap_tag = tags::load_tag('ltmp', bsp_ref.structureLightmap.tag_name, 7);

		auto bsp = tags::get_tag<scenario_structure_bsp_block>('sbsp', bsp_tag);
		auto lightmap = tags::get_tag<scenario_structure_lightmap_block>('ltmp', lightmap_tag);

		if (LOG_CHECK(lightmap->lightmapGroups.size > 0))
		{
			auto *lightmap_group = lightmap->lightmapGroups[0];
			printf("Copying cluster data...");
			for (size_t i = 0; i < lightmap_group->clusters.size; i++)
			{
				auto *lightmap_cluster = lightmap_group->clusters[i];
				auto *bsp_cluster = bsp->clusters[i];
				tags::block_delete_all(&lightmap_cluster->cacheData);
				tags::copy_block(&bsp_cluster->clusterData, &lightmap_cluster->cacheData);
			}
			printf("done\n");

			printf("Copying instance geo data...");
			for (size_t i = 0; i < lightmap_group->poopDefinitions.size; i++)
			{
				auto *instance_geo_lightmap = lightmap_group->poopDefinitions[i];
				auto *bsp_instance_geo = bsp->instancedGeometriesDefinitions[i];
				tags::block_delete_all(&instance_geo_lightmap->cacheData);
				tags::copy_block(&bsp_instance_geo->renderInfo.renderData, &instance_geo_lightmap->cacheData);
			}
			printf("done\n");
			
		}
		tags::save_tag(lightmap_tag);
		tags::unload_tag(bsp_tag);
		tags::unload_tag(lightmap_tag);
	}

	printf("=== Stage 1 complete ===\n");
}

const s_tool_command_argument lightmaps_fix_args[] =
{
	{ _tool_command_argument_type_tag_name, L"scenario", "*.scenario" }
};

static const s_tool_command fix_extraced_lightmap
{
	L"fix extracted lightmaps",
	fix_extracted_lightmaps,
	lightmaps_fix_args,
	ARRAYSIZE(lightmaps_fix_args),
	true
};

static void _cdecl dump_tag_as_xml_proc(const wchar_t *argv[])
{
	blam_tag tag_type;
	std::string tag_path = filesystem_path_to_tag_path(argv[0], &tag_type);
	if (tag_type.is_none())
	{
		printf("Unknown tag type\n");
		return;
	}
	printf("%s : %s\n", tag_path.c_str(), tag_type.as_string().c_str());
	datum tag = tags::load_tag(tag_type, tag_path, 7);

	if (!tag.is_valid())
	{
		printf("Can't load tag!\n");
		return;
	}

	std::string dump_file_name = get_full_tag_path(tag_path) + "." + H2CommonPatches::tag_group_names.at(tag_type.as_int());
	std::string dump_file_path = dump_file_name.substr(0, dump_file_name.find_last_of("\\"));

	int error_code = SHCreateDirectoryExA(NULL, dump_file_path.c_str(), NULL);
	ASSERT_CHECK(error_code == ERROR_SUCCESS || error_code == ERROR_ALREADY_EXISTS || error_code == ERROR_FILE_EXISTS);

	TagDumper::dump_as_xml(tag, dump_file_name);
}

const s_tool_command_argument dump_as_xml_args[] =
{
	{ _tool_command_argument_type_tag_name, L"tag" }
};

static const s_tool_command dump_as_xml
{
	L"dump as xml",
	dump_tag_as_xml_proc,
	dump_as_xml_args,
	ARRAYSIZE(dump_as_xml_args),
	true
};

static bool fixed_invalid;
static void __cdecl fixed_invalid_fields_hook()
{
	fixed_invalid = true;
}

static void _cdecl fix_extracted_bitmaps(const wchar_t *argv[])
{
	PatchCall(0x0531EF2, fixed_invalid_fields_hook);
	auto start_time = std::chrono::high_resolution_clock::now();
	std::cout << "Scanning for bitmaps in tag folders" << std::endl;
	std::unordered_set<std::string> bitmaps;

	auto scan_tag_folder = [&](const std::string &path)
	{
		std::cout << path << std::endl;
		for (auto& p : fs::recursive_directory_iterator(path + "\\tags\\"))
		{
			std::string ext = p.path().extension().string();
			if (ext == ".bitmap")
			{
				std::string bitmap_path = p.path().parent_path().generic_string() + "\\" + p.path().stem().generic_string();
				std::replace(bitmap_path.begin(), bitmap_path.end(), '/', '\\');
				bitmaps.insert(bitmap_path);
			}
		}
	};
	scan_tag_folder(H2CommonPatches::get_h2ek_documents_dir());
	scan_tag_folder(process::GetExeDirectoryNarrow());

	std::cout << bitmaps.size() << " tag(s) found" << std::endl
		<< "Please wait this may take a while" << std::endl;

	std::vector<std::string> tags_fixed;
	for (const std::string &bitmap_path : bitmaps)
	{
		fixed_invalid = false;
		datum tag = tags::load_tag('bitm', bitmap_path, 7);
		if (!tag.is_valid())
			continue;
		auto bitmap = tags::get_tag<bitmap_block>('bitm', tag);
		if (bitmap->bitmaps.size < 1)
		{
			std::cout << "wtf: bitmap tag has no bitmaps in it!" << std::endl 
				<< "tag path:" << bitmap_path;
			continue;
		}

		if (fixed_invalid)
		{
			tags_fixed.push_back(bitmap_path);
			tags::save_tag(tag);
		}
		tags::unload_tag(tag);
	}
	std::cout << tags_fixed.size() << " tags fixed" << std::endl;
	
	std::ofstream fixed_bitmaps("bitmaps_fixed.txt");
	for (const std::string &tag : tags_fixed)
		fixed_bitmaps << tag << std::endl;
	std::cout << "File list saved to bitmaps_fixed.txt" << std::endl;

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time_taken = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
	std::string time_taken_human = beautify_duration(time_taken);
	printf("== Time taken: %s ==", time_taken_human.c_str());
}

static const s_tool_command fix_extracted_bitmap_tags
{
	L"fix extracted bitmaps",
	fix_extracted_bitmaps,
	nullptr,
	0,
	true
};

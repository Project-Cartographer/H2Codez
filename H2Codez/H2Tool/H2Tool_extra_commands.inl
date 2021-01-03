#include "stdafx.h"
#include "H2tool.h"
#include "H2ToolLibrary.inl"
#include "LightMapping.h"
#include "Common/RenderGeometryDumper.h"
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
#include "Tags/RenderModel.h"
#include "util/Patches.h"
#include "util/process.h"
#include "util/FileSystem.h"
#include <iostream>
#include <sstream>
#include <codecvt>
#include <unordered_set>
#include <direct.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#pragma region util

/*
	Show a CMD prompt to the user
	Returns user response or default
*/
inline static bool prompt_user(const std::string& message, bool default = false)
{
	std::cout << message << " (Y/N) [Default:" << (default ? "Y" : "N") << "]" << std::endl;
	
	std::string input;
	std::cin >> input;
	str_trim(input);
	input = tolower(input);
	if (input.size() >= 1)
	{
		if (input[0] == 'y')
		{
			return true;
		}
		else if (input[0] == 'n')
		{
			return false;
		}
	}
	return default;
}

/*
	Show a CMD prompt to the user waiting till user gives valid response
	Returns user response
*/
inline static bool prompt_user_wait(const std::string& message)
{
	std::cout << message << " (Y/N)" << std::endl;
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
				return true;
			}
			else if (input[0] == 'n')
			{
				return false;
			}
		}
	}
	abort(); // unreachable
}

/*
	Convert file-system path to tag path + type
*/
static std::string filesystem_path_to_tag_path(const wchar_t *fs_path, blam_tag *tag_type = nullptr)
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

#pragma endregion util

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

/* Returns a pointer to the utility command table */
inline static s_tool_h2dev_command *get_tag_utility_command_table()
{
	return reinterpret_cast<s_tool_h2dev_command*>(0x97A910);
}

/* Should we hide the ulitiy command from the end user because it doesn't work */
inline static bool should_filter_command(size_t id)
{
	// black-listed proc offsets
	DWORD bad_command_impl[] = {
		0x5B8EC0, // ret false
		0x565470, // ret true
		0x403480, // devcmd_null_proc
		// maybe these are useful in other modes?
		0x401BC0, // devcmd_find_old_objects
		0x403390, // devcmd_remove_huds
	};
	auto command = get_tag_utility_command_table()[id];
	if (array_util::contains(bad_command_impl, reinterpret_cast<DWORD>(command.command_impl)))
		return true;
	return false;
}

static s_tool_h2dev_command *get_tag_utility_command_by_name(wcstring W_function_name)
{
	std::string function_name = tolower(wstring_to_string.to_bytes(W_function_name));
	s_tool_h2dev_command *command_table = get_tag_utility_command_table();
	for (int i = 0; i <= extra_commands_count; i++) {
		s_tool_h2dev_command *current_cmd = &command_table[i];
		if (function_name == current_cmd->name)
			return current_cmd;
	}
	return nullptr;
}

void _cdecl list_all_extra_commands_proc(wcstring* arguments)
{
	s_tool_h2dev_command *command_table = get_tag_utility_command_table();
	printf("\n  help : " help_desc);
	printf("\n  list all : " list_all_desc);
	for (int i = 0; i <= extra_commands_count; i++) {
		if (!is_debug_build() && should_filter_command(i))
			continue;
		s_tool_h2dev_command *current_cmd = (command_table + i);
		printf("\n  %s <%s> : %s", current_cmd->name, H2CommonPatches::tag_group_names.at(current_cmd->tag_type.as_int()).c_str(), current_cmd->description);
	}
}

inline static void extra_commands_help(const wchar_t* command)
{
	s_tool_h2dev_command *cmd = get_tag_utility_command_by_name(command);
	if (cmd) {
		printf("\n  usage : %s\n  Description : %s\n", cmd->name, cmd->description);
	}
	else {
		if (_wcsicmp(command, L"help") == 0) {
			printf("\n  usage : help\n  Description : " help_desc);
		}
		else if (_wcsicmp(command, L"list") == 0) {
			printf("\n  usage : list all\n  Description : " list_all_desc);
		}
		else {
			printf("\n  No such command \"<command_name>\"");
			printf("\n  usage : extra-commands help <command_name>\n  Description : Prints the information of the <command_name>");
		}
	}
}

enum save_settings
{
	prompt,
	silent_save,
	nosave
};

/*
	Executes a utility command, returns success, errors are logged to log or console
	Set save_settings to control tag save behaviour
*/
static bool execute_utility_command(s_tool_h2dev_command *command, const std::string &tag_name, save_settings save_setting = prompt)
{
	std::cout << "Tag: \"" << tag_name << "\"" << std::endl;
	datum tag = tags::load_tag(command->tag_type, tag_name.c_str(), tags::skip_child_tag_load);

	if (!tag.is_valid() && !is_debug_build())
	{
		printf("\n Error unable to find tag \"%s\"!", tag_name.c_str());
		return false;
	}
	LOG_FUNC("%s : %s : %s : %s", command->name, tag_name.c_str(), command->tag_type.as_string().c_str(), tags::get_name(tag));
	flushall(); // flush the console incase the command crashes
	if (command->command_impl(tag_name.c_str(), tag))
	{
		if (save_setting == silent_save ||
				save_setting != nosave && prompt_user_wait("Do you want to save the tag (\"" + tag_name + "\")?"))
			tags::save_tag(tag);
	}
	else {
		LOG_FUNC("skipping saving (command proc returned false)");
	}

	if (tag.is_valid())
		tags::unload_tag(tag);
	return true;
}

static void _cdecl h2dev_extra_commands_proc(const wchar_t ** arguments)
{
	const wchar_t *command_name = arguments[0];
	const wchar_t *command_parameter = arguments[1];

	if (_wcsicmp(command_name, L"list") == 0) {
		list_all_extra_commands_proc(nullptr);
	} else if (_wcsicmp(command_name, L"help") == 0) {
		extra_commands_help(command_parameter);
	} else {
		s_tool_h2dev_command *cmd = get_tag_utility_command_by_name(command_name);
		if (!cmd) {
			printf("\n  No such command present.");
			printf("\n  See extra-commands-list");
		} else {
			printf("\nRunning command %ws\n", command_name);
			std::string tag_name = wstring_to_string.to_bytes(command_parameter);
			execute_utility_command(cmd, tag_name);
		}
	}
}

static void _cdecl h2dev_extra_iterate_command_proc(const wchar_t** arguments)
{
	const wchar_t* command_name = arguments[0];

	if (_wcsicmp(command_name, L"list") == 0 || _wcsicmp(command_name, L"help") == 0) {
		list_all_extra_commands_proc(nullptr);
	} else {
		s_tool_h2dev_command* cmd = get_tag_utility_command_by_name(command_name);
		if (!cmd) {
			printf("\n  No such command present.");
			printf("\n  See extra-commands-list");
		}
		else {
			
			std::unordered_set<std::string> tag_paths;
			auto ext = H2CommonPatches::tag_group_names.at(cmd->tag_type.as_int());
			auto scan_tag_folder = [&](const std::string& path)
			{
				find_all_files_with_extension(tag_paths, path + "\\tags\\", ext);
			};

			scan_tag_folder(H2CommonPatches::get_h2ek_documents_dir());
			scan_tag_folder(process::GetExeDirectoryNarrow());

			std::cout << tag_paths.size() << " tag(s) found" << std::endl;

			save_settings save_mode = save_settings::prompt;
			if (!prompt_user("Show tag save prompt for all tags?", true))
				save_mode = (prompt_user("Save tags?") ? save_settings::silent_save : save_settings::nosave);

			std::cout << "Command: " << cmd->name << "\t" "Tag type: " << ext << "\t" "Count: " << tag_paths.size()
				<< std::endl << "Autosave modified tag?: " << (save_mode == save_settings::silent_save ? "ENABLED!" : "Disabled") << std::endl;

			if (prompt_user_wait("About to run on a command on " + std::to_string(tag_paths.size()) + " tag(s). Do you wish to continue?"))
			{
				for (const auto &tag: tag_paths)
					LOG_CHECK(execute_utility_command(cmd, tag, save_mode));
			}
		}
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
	h2dev_extra_commands_proc,
	h2dev_extra_commands_arguments,	NUMBEROF(h2dev_extra_commands_arguments),
	false
};

static const s_tool_command_argument h2dev_extra_iterate_command[] = {
	{
		_tool_command_argument_type_string,
		L"command name",
	}
};

static const s_tool_command h2dev_extra_iterate_commands_defination = {
	L"extra commands iterate",
	h2dev_extra_iterate_command_proc,
	h2dev_extra_iterate_command,	NUMBEROF(h2dev_extra_iterate_command),
	false
};

static const s_tool_command list_extra_commands = {
	L"extra commands list",
	list_all_extra_commands_proc,
	nullptr, 0,
	true
};

#pragma endregion
#pragma region Render_model_import

static std::vector<std::unique_ptr<tags::s_scoped_handle>> imported_bsps;
static bool in_fake_structure_compile = false;
static bool structure_imported = false;

static DWORD __cdecl tag_save__scenario_for_structure_import(int TAG_INDEX)
{
	if (in_fake_structure_compile)
		return true;
	else
		return tags::save_tag(TAG_INDEX);
}

static DWORD __cdecl tag_save__structure_for_structure_import(int TAG_INDEX)
{
	bool result = true;
	if (in_fake_structure_compile) {
		structure_imported = true;
		imported_bsps.push_back(std::move(std::make_unique<tags::s_scoped_handle>(TAG_INDEX)));
		if (is_debug_build())
			tags::save_tag(TAG_INDEX);
	} else {
		result = tags::save_tag(TAG_INDEX);
	}
	return result;
}

static void __cdecl tag_unload__structure_for_structure_import(int TAG_INDEX)
{
	if (!in_fake_structure_compile)
		tags::unload_tag(TAG_INDEX);
}

static inline bool import_structure_for_render_model(std::wstring scenario_path, std::wstring bsp_name, bool is_jms)
{
	structure_imported = false;
	wcstring fake_args[2] = { scenario_path.c_str(), bsp_name.c_str() };
	auto flags = import_flags_reimport | import_flags_skip_objects;
	if (is_debug_build())
		flags |= (import_flags_debug_b | import_flags_debug_a);
	if (!is_jms)
		flags |= import_flags_ass_file;

	in_fake_structure_compile = true;
	import_structure_main(fake_args, flags);
	in_fake_structure_compile = false;

	return structure_imported;
}

static void _cdecl TAG_RENDER_MODEL_IMPORT_PROC(file_reference *file, const datum &tag)
{
	render_model_block *render_model = tags::get_tag<render_model_block>('mode', tag);
	ASSERT_CHECK(in_fake_structure_compile == false);

	if (LOG_CHECK(tag != datum::null())) {
		if (TAG_ADD_IMPORT_INFO_ADD_DATA_(&render_model->importInfo, file))
		{
			printf("    == Import info  Added \n");
		} else {
			printf("    == Failed to add import info");
			return;
		}

		std::string path      = FiloInterface::get_path_info(file, PATH_FLAGS::CONTAINING_DIRECTORY_PATH);
		std::string file_type = FiloInterface::get_path_info(file, PATH_FLAGS::FILE_EXTENSION);
		std::string file_name = FiloInterface::get_path_info(file, PATH_FLAGS::FILE_NAME);

		remove_last_part_of_path(&path[0]);
		std::wstring wide_path = wstring_to_string.from_bytes(path.c_str());
		std::wstring wide_name = wstring_to_string.from_bytes(file_name);

		bool imported = import_structure_for_render_model(wide_path, wide_name, file_type == "jms");

		if (!imported)
			printf("    == failed to import geometry \n   ===SKIPPPING %s \n", path.c_str());

		printf("    == leaving TAG_RENDER_MODEL_IMPORT_PROC\n");
	}
}
static const s_tool_import_definations TAG_RENDER_IMPORT_DEFINATIONS_[] = {
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

enum cluster_info_type : BYTE
{
	none,
	havok,
	visibility
};
// Controls what sort of extra info is generated for the cluster
inline static void set_cluster_info_for_sbsp(cluster_info_type type)
{
	WriteValue<BYTE>(0x478A6F + 1, type);
}

// Controls whatever BSP is compiled as triangle strip
inline static void toggle_triangle_strip_for_sbsp(bool use_triangle_strip)
{

	WriteValue<BYTE>(0x41E82A + 1, use_triangle_strip ? 0x0 : 0x1);
	WriteValue<BYTE>(0x41E82E + 1, use_triangle_strip);
}

static void *jms_collision_geometry_import_defination_ = CAST_PTR(void*, 0x97C350);
static bool _cdecl h2pc_generate_render_model(datum tag, file_reference& FILE_REF)
{
	/* Patches */

	//replacing 'structure' folder text with 'render' folder
	WritePointer(0x41F52D, "render");

	// hook tag save and unload functions to capture tag info
	PatchCall(0x41CDE5, tag_save__scenario_for_structure_import);
	PatchCall(0x41FEFE, tag_save__structure_for_structure_import);
	PatchCall(0x4200BD, tag_unload__structure_for_structure_import);

	// switch to triangle strip if required 
	bool use_triangle_strip = conf.exists("use_triangle_strip") ? conf.getBoolean("use_triangle_strip") : true;
	toggle_triangle_strip_for_sbsp(use_triangle_strip);
	set_cluster_info_for_sbsp(use_triangle_strip ? none : visibility);

	auto default_string_id = string_id::find_by_name("default");

	// Did import and conversion succeed?
	bool success = false;
	std::string model_name = get_path_filename(tags::get_name(tag));
	render_model_block* render_model = ASSERT_CHECK(tags::get_tag<render_model_block>('mode', tag));

	// fix flags and set name
	render_model->flags |= render_model->ForceNodeMaps;
	render_model->name = string_id::get_string_id(model_name);

	// add section group
	if (render_model->sectionGroups.size == 0)
	{
		printf("        ### Adding section group\n");
		auto idx = tags::add_block_element(&render_model->sectionGroups);
		auto *group = render_model->sectionGroups[idx];
		group->detailLevels = render_model_section_group_block::All;
	}

	// ensure we have at least one region and permutation or things will crash

	if (render_model->regions.size == 0)
		tags::resize_block(&render_model->regions, 1);

	auto region = render_model->regions[0];
	if (region->nodeMapSizeOLD == 0)
		region->nodeMapOffsetOLD = NONE;
	if (region->permutations.size == 0)
		tags::resize_block(&region->permutations, 1);

	if (load_model_object_definations_(&render_model->importInfo, jms_collision_geometry_import_defination_, 1, FILE_REF))
	{
		if (TAG_ADD_IMPORT_INFO_BLOCK(&render_model->importInfo))
		{
			use_import_definitions(TAG_RENDER_IMPORT_DEFINATIONS_, ARRAYSIZE(TAG_RENDER_IMPORT_DEFINATIONS_), FILE_REF, &tag, 0);

			// clear old sections and materials as we are replacing them
			tags::block_delete_all(&render_model->sections);
			tags::block_delete_all(&render_model->materials);

			render_model_node_block empty_node
			{
				default_string_id,
				NONE, NONE, NONE,
				0,
				{0, 0, 0},
				{0, 0, 0, 1},
				{{0, 0, 0, 1}},
				0
			};

			auto empty_node_idx = render_model->nodes.find_element(
				[empty_node](const render_model_node_block *node) 
				{ return memcmp(&node->parentNode, &empty_node.parentNode, sizeof(render_model_node_block) - offsetof(render_model_node_block, parentNode)) == 0; }
			);

			if (empty_node_idx == NONE) {
				empty_node_idx = tags::add_block_element(&render_model->nodes);
				*render_model->nodes[empty_node_idx] = empty_node;
			}

			for (const auto& tag : imported_bsps) {
				std::cout << "Converting BSP: " << tags::get_name(tag->get_tag_datum()) << std::endl;
				auto *sbsp = tags::get_tag<scenario_structure_bsp_block>('sbsp', tag->get_tag_datum());
				if (sbsp->clusters.size != 1)
				{
					printf("cluster count needs to be %d but is %d ", 1, sbsp->clusters.size);
					success = false;
				} else {
					int section_idx = tags::add_block_element(&render_model->sections);
					auto section = render_model->sections[section_idx];
					auto cluster = sbsp->clusters[0];

					section->sectionInfo = cluster->sectionInfo;
					ASSERT_CHECK(section->sectionInfo.bounds.size == 0);

					// set ridge node to the empty node
					section->rigidNode = static_cast<short>(empty_node_idx);

					// fix geo classification
					section->sectionInfo.geometryClassification = GeometryClassification::Rigid;
					section->classification = GeometryClassification::Rigid;
					
					// add section data
					tags::resize_block(&section->sectionData, 1);

					if (ASSERT_CHECK(section->sectionData.size == 1) && LOG_CHECK(cluster->clusterData.size == 1))
					{
						// copy data from the cluster to the section

						auto *section_data = section->sectionData[0];
						auto *cluster_data = cluster->clusterData[0];
#define copy_block(name) tags::copy_block(&cluster_data->##name, &section_data->section.##name)
						copy_block(parts);
						copy_block(subparts);
						copy_block(visibilityBounds);
						copy_block(rawVertices);
						copy_block(stripIndices);
						copy_block(vertexBuffers);
#undef copy_block
						// fix materials index
						for (auto& part : section_data->section.parts)
							part.material += static_cast<short>(render_model->materials.size);

						// copy over materials for this BSP
						tags::copy_block(&sbsp->materials, &render_model->materials);

						// add node map entry
						tags::resize_block(&section_data->nodeMap, 1);
						section_data->nodeMap[0]->nodeIndex = static_cast<byte>(empty_node_idx);

						success = true;
					}
				}
			}
		}
	}

	// isn't it a good idea to do it before we crash on exit?
	imported_bsps.clear();

	// restore import folder
	WritePointer(0x41F52D, "structure");

	// reset triangle strip
	toggle_triangle_strip_for_sbsp(false);
	set_cluster_info_for_sbsp(havok);

	in_fake_structure_compile = false;

	return success;
}

static bool _cdecl h2pc_import_render_model_proc(wcstring* arguments)
{
	file_reference file_reference;
	wchar_t wide_tag_path[256];
	static WCHAR out_path[256];

	if (tool_build_paths(arguments[0], "render", file_reference, out_path, &wide_tag_path))
	{
		std::string tag_path = wstring_to_string.to_bytes(wide_tag_path);
		tags::s_scoped_handle tag = tags::load_tag('mode', tag_path, 7);
		if (tag.is_valid())
		{
			render_model_block *render_model = tags::get_tag<render_model_block>('mode', tag);

			if (!load_model_object_definations_(&render_model->importInfo, jms_collision_geometry_import_defination_, 1, file_reference))
				return false;

		} else {
			printf("        ### creating new render model file with name '%ws' \n ", wide_tag_path);
			tag = tags::new_tag('mode', tag_path);
		}

		if (LOG_CHECK(tag.is_valid()))
		{
			if (TAG_FILE_CHECK_IS_WRITEABLE(tag, false))
			{
				if (h2pc_generate_render_model(tag, file_reference))
				{
					printf("saving tag: %s\n", tags::get_name(tag).c_str());
					tags::save_tag(tag);
					return true;
				}
				else
				{
					printf("      ### FATAL ERROR unable to generate render model '%ws' \n", wide_tag_path);
					return false;
				}
			}
			else
			{
				printf("      ### ERROR render model '%ws' is not writable\n", wide_tag_path);
				return false;
			}
		}
		else
		{
			printf("     ### ERROR unable to create render model '%ws'\n", wide_tag_path);
			return false;
		}

	}
	else
	{
		wprintf(L"### ERROR unable to find 'render' data directory for '%s' ", arguments[0]);
		return false;
	}
}

#pragma endregion
#pragma region pathfinding

/*
	Check if a sbsp DOESN'T have pathfinding data and prompt user otherwise
*/
static bool check_pathfinding_clear(scenario_structure_bsp_block *target)
{
	if (target->pathfindingData.size > 0)
	{
		if (prompt_user_wait("bsp already has pathfinding. Do you want to overwrite it?"))
		{
			std::cout << "Clearing old pathfinding data" << std::endl;
			tags::block_delete_all(&target->pathfindingData);
			return true;
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

#pragma endregion
#pragma region lightmap
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
			for (int32_t i = 0; i < lightmap_group->clusters.size; i++)
			{
				auto *lightmap_cluster = lightmap_group->clusters[i];
				auto *bsp_cluster = bsp->clusters[i];
				tags::block_delete_all(&lightmap_cluster->cacheData);
				tags::copy_block(&bsp_cluster->clusterData, &lightmap_cluster->cacheData);
			}
			printf("done\n");

			printf("Copying instance geo data...");
			for (int32_t i = 0; i < lightmap_group->poopDefinitions.size; i++)
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
#pragma endregion
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
	datum tag = tags::load_tag(tag_type, tag_path, tags::for_editor | tags::skip_child_tag_load | tags::skip_tag_postprocess | tags::skip_block_postprocess);

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


static const wchar_t* __cdecl create_bitmap_from_other_image(file_reference* file, bitmap_data_block** bitmap_out)
{
	typedef const wchar_t* __cdecl create_bitmap_from_other_image(file_reference* a1, bitmap_data_block** a2);
	auto impl = reinterpret_cast<create_bitmap_from_other_image*>(0x4E7840);
	return impl(file, bitmap_out);
}

static void bitmap_insert_at_index(bitmap_block* bitmap_tag, int index, bitmap_data_block* new_bitmap) {
	typedef void __cdecl bitmap_insert_at_index(bitmap_block* bitmap_tag, int index, bitmap_data_block* new_bitmap);
	auto impl = reinterpret_cast<bitmap_insert_at_index*>(0x53A7D0);
	impl(bitmap_tag, index, new_bitmap);
}

static void bitmap_remove_by_index(bitmap_block* bitmap, signed int index) {
	typedef void __cdecl bitmap_remove_by_index(bitmap_block* bitmap, signed int index);
	auto impl = reinterpret_cast<bitmap_remove_by_index*>(0x53A5B0);
	impl(bitmap, index);
}

static void free_bitmap_data_block(bitmap_data_block* bitmap) {
	typedef void __cdecl free_bitmap_data_block(bitmap_data_block* a1);
	auto impl = reinterpret_cast<free_bitmap_data_block*>(0x71E5D0);
	impl(bitmap);
}

static void _cdecl edit_bitmap_proc(const wchar_t* argv[])
{
	auto bitmap_name = wstring_to_string.to_bytes(argv[0]);
	tags::s_scoped_handle bitmap_tag = ASSERT_CHECK(tags::load_tag('bitm', bitmap_name, tags::for_editor));
	auto bitmap = ASSERT_CHECK(tags::get_tag<bitmap_block>('bitm', bitmap_tag));
	auto file_ref = file_reference(wstring_to_string.to_bytes(argv[2]), false);
	bitmap_data_block *out = nullptr;
	auto error = create_bitmap_from_other_image(&file_ref, &out);
	if (error) {
		wcout << error << endl;
		return;
	}
	ASSERT_CHECK(out);
	auto index = _wtoi(argv[1]);
	bitmap_remove_by_index(bitmap, index);
	bitmap_insert_at_index(bitmap, index, out);
	tags::save_tag(bitmap_tag);

	free_bitmap_data_block(out);
}

const s_tool_command_argument edit_bitmap_args[] =
{
	{ _tool_command_argument_type_tag_name, L"bitmap" },
	{ _tool_command_argument_type_string, L"index" },
	{ _tool_command_argument_type_data_file, L"replacement" },
};


static const s_tool_command edit_bitmap
{
	L"edit bitmap",
	edit_bitmap_proc,
	edit_bitmap_args,
	ARRAYSIZE(edit_bitmap_args),
	false
};

static void _cdecl structure_dump_proc(const wchar_t* argv[])
{
	auto bsp_path = wstring_to_string.to_bytes(argv[0]);
	auto dae_dump_path = wstring_to_string.to_bytes(argv[1]);

	tags::s_scoped_handle bsp_tag = load_tag_no_processing('sbsp', bsp_path);
	if (!bsp_tag)
		return;
	try {
		auto structure = ASSERT_CHECK(tags::get_tag<scenario_structure_bsp_block>('sbsp', bsp_tag));
		RenderModel2COLLADA dump(structure->materials, false);

		cout << "Exporting clusters.." << endl;

		std::vector<const global_geometry_section_struct_block*> clusters;
		for (const auto & cluster : structure->clusters)
			clusters.push_back(ASSERT_CHECK(cluster.clusterData[0]));

		auto bsp_name = get_path_filename(bsp_path);
		auto main_geo = dump.AddMutlipleSections(bsp_name, clusters);
		dump.AddSectionInstance(main_geo, bsp_name);

		cout << "Exporting instances.." << endl;

		std::vector<RenderModel2COLLADA::MESH_ID> instance_meshes;
		for (auto i = 0; i < structure->instancedGeometriesDefinitions.size; i++) {
			auto instance = ASSERT_CHECK(structure->instancedGeometriesDefinitions[i]);
			auto section = ASSERT_CHECK(instance->renderInfo.renderData[0]);

			auto name = "instance_mesh_" + std::to_string(i);
			instance_meshes.push_back(dump.AddSection("", section));
		}

		for (auto i = 0; i < structure->instancedGeometryInstances.size; i++) {
			auto instance = ASSERT_CHECK(structure->instancedGeometryInstances[i]);
			auto name = "%" + std::string(instance->name.get_name());

			// todo(num0005) this shouldn't be required
			auto transform = instance->transform;
			transform.inverse_rotation();

			dump.AddSectionInstance(instance_meshes[instance->instanceDefinition], name, transform);
		}

		cout << "Exporting portals.." << endl;

		int portal_index = 0;
		for (const auto& structure_portal : structure->clusterPortals) {
			ASSERT_CHECK(structure_portal.vertices.size >= 2);

			COLLADA::Mesh portal_mesh;
			auto base_trig = portal_mesh.vertices.size();
			COLLADA::Mesh::Part part;
			part.material = "%portal";

			size_t index = 0;
			std::vector<int> indices_left;
			indices_left.reserve(structure_portal.vertices.size);

			for (const real_point3d& vert : structure_portal.vertices) {
				portal_mesh.vertices.push_back({ vert.x, vert.y, vert.z });
				indices_left.push_back(indices_left.size());
			}

			// taken from https://github.com/Project-Cartographer/H2PC_TagExtraction/blob/39ccb685331a1f651693352247d5412ebb96320a/BlamLib/BlamLib/Render/COLLADA/Export/ColladaExporter.cs#L252
			for (auto i = 0; i < structure_portal.vertices.size - 2; i++) {
				COLLADA::Mesh::Triangle triangle;
				triangle.vertex_list[0] = base_trig + indices_left[index + 0];
				triangle.vertex_list[1] = base_trig + indices_left[index + 1];
				triangle.vertex_list[2] = base_trig + indices_left[index + 2];

				indices_left.erase(indices_left.begin() + index + 1); // remove middle element

				if (index + 3 < indices_left.size())
					index++;
				else
					index = 0;

				part.triangles.push_back(triangle);
			}
			portal_mesh.parts.push_back(part);
			auto id = std::to_string(portal_index++);
			auto portal_mesh_id = dump.GetCollada().AddMesh("portal_mesh_" + id, portal_mesh);
			dump.AddSectionInstance(portal_mesh_id, "portal_" + id);
		}

		dump.Write(dae_dump_path);
		cout << "Saved to " << dae_dump_path << endl;
	}
	catch (const std::exception& ex) {
		cout << "Exception: " << ex.what() << endl;
	}
}

const s_tool_command_argument structure_2_dae[] =
{
	{ _tool_command_argument_type_tag_name, L"BSP" },
	{ _tool_command_argument_type_data_file, L"DAE" },
};


static const s_tool_command structure_dump
{
	L"structure 2 dae",
	structure_dump_proc,
	structure_2_dae,
	ARRAYSIZE(structure_2_dae),
	false
};


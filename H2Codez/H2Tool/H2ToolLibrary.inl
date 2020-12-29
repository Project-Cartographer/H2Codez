#pragma once
#include "stdafx.h"
#include "Common/FiloInterface.h"
#include "Common/BasicTagTypes.h"
#include "Common/TagInterface.h"

static const char* get_h2tool_version()
{
	typedef char*(_cdecl* _get_h2tool_build_date)();
	static _get_h2tool_build_date get_h2tool_build_date = CAST_PTR(_get_h2tool_build_date, 0xEA760);

	return get_h2tool_build_date();
}

static bool __cdecl TAG_ADD_IMPORT_INFO_BLOCK(tag_block_ref *import_info)
{
	typedef bool(_cdecl* TAG_ADD_IMPORT_INFO_BLOCK)(tag_block_ref*);
	static TAG_ADD_IMPORT_INFO_BLOCK _TAG_ADD_IMPORT_INFO_BLOCK = CAST_PTR(TAG_ADD_IMPORT_INFO_BLOCK, 0x545D40);

	return _TAG_ADD_IMPORT_INFO_BLOCK(import_info);
}
static bool __cdecl TAG_ADD_IMPORT_INFO_ADD_DATA_(tag_block_ref *IMPORT_INFO_OFFSET, file_reference *FILE_REF)
{
	typedef bool(_cdecl* TAG_ADD_IMPORT_INFO_ADD_DATA_)(tag_block_ref*, file_reference*);
	static TAG_ADD_IMPORT_INFO_ADD_DATA_ _TAG_ADD_IMPORT_INFO_ADD_DATA_ = CAST_PTR(TAG_ADD_IMPORT_INFO_ADD_DATA_, 0x545E90);

	return _TAG_ADD_IMPORT_INFO_ADD_DATA_(IMPORT_INFO_OFFSET, FILE_REF);
}
static bool __cdecl TAG_FILE_CHECK_IS_WRITEABLE(datum TAG, bool suppress_error = false)
{
	typedef bool(_cdecl* _TAG_FILE_CHECK_READ_ONLY_ACCESS)(int, int);
	static _TAG_FILE_CHECK_READ_ONLY_ACCESS TAG_FILE_CHECK_READ_ONLY_ACCESS = CAST_PTR(_TAG_FILE_CHECK_READ_ONLY_ACCESS, 0x410D20);

	return TAG_FILE_CHECK_READ_ONLY_ACCESS(TAG.as_long(), suppress_error);
}

static int __cdecl TAG_BLOCK_GET_ELEMENT_WITH_SIZE(void* FIELD_OFFSET, int ELEMENT_INDEX, int FIELD_SIZE)
{
	typedef int(_cdecl* TAG_BLOCK_GET_ELEMENT_WITH_SIZE)(void*, int, int);
	static TAG_BLOCK_GET_ELEMENT_WITH_SIZE TAG_BLOCK_GET_ELEMENT_WITH_SIZE_ = CAST_PTR(TAG_BLOCK_GET_ELEMENT_WITH_SIZE, 0x532970);

	return TAG_BLOCK_GET_ELEMENT_WITH_SIZE_(FIELD_OFFSET, ELEMENT_INDEX, FIELD_SIZE);
}

typedef bool (*find_tag_comparison)(void *element, void *find_data);

static char __cdecl load_model_object_definations_(tag_block_ref *IMPORT_INFO_OFFSET_, void *proc_definations_, __int16 proc_count, file_reference& FILE_REF_)
{
	typedef char(_cdecl* _load_model_object_definations_)(tag_block_ref *, void *, __int16, file_reference&);
	static _load_model_object_definations_ load_model_object_definations_ = CAST_PTR(_load_model_object_definations_, 0x412E00);

	return load_model_object_definations_(IMPORT_INFO_OFFSET_, proc_definations_, proc_count, FILE_REF_);
}
static bool _cdecl tool_build_paths(wcstring directory, const char* Subfolder, file_reference& out_reference, wchar_t out_path[256], wchar_t (*tag_path)[256] = nullptr)
{
	typedef bool(_cdecl*_tool_build_paths)(wcstring, const char*, file_reference&, wchar_t[256], void*);
	static const _tool_build_paths tool_build_paths = CAST_PTR(_tool_build_paths, 0x4119B0);
	return tool_build_paths(directory, Subfolder, out_reference, out_path, tag_path);
}
static bool _cdecl use_import_definitions(const s_tool_import_definations* definitions, int count, file_reference& reference, void* context_data, bool unk = false)
{
	typedef bool(_cdecl* _use_import_definitions)(const s_tool_import_definations*, int, file_reference&, void*, DWORD);
	static const _use_import_definitions use_import_definitions = CAST_PTR(_use_import_definitions, 0x412100);
	return use_import_definitions(definitions, count, reference, context_data, unk);
}

enum import_flags : __int32
{
	import_flags_none = 0x0,
	import_flags_debug_a = 0x1,
	import_flags_debug_b = 0x2,
	import_flags_4 = 0x4,
	import_flags_plane_debug_generate = 0x8,
	import_flags_plane_debug = 0x10,
	import_flags_ass_file = 0x20,
	import_flags_40 = 0x40,
	import_flags_skip_objects = 0x80,
	import_flags_reimport = 0x100,
	import_flags_preserve_old = 0x200,
	import_flags_visibility = 0x400,
	import_flags_simulate = 0x800,
};

inline import_flags operator|(import_flags a, import_flags b)
{
	return static_cast<import_flags>(static_cast<__int32>(a) | static_cast<__int32>(b));
}

inline import_flags &operator|=(import_flags &a, import_flags b)
{
	a = a | b;
	return a;
}


// args: scenario name, bsp name
static void import_structure_main(const wchar_t **args, import_flags flags)
{
	typedef void __cdecl import_structure(const wchar_t **args, __int32 flags);
	auto import_structure_impl = reinterpret_cast<import_structure*>(0x41F4D0);
	import_structure_impl(args, flags);
}

static inline tags::s_scoped_handle load_tag_no_processing(blam_tag type, const std::string& name) {
	tags::s_scoped_handle tag = tags::load_tag(type, name, tags::skip_child_tag_load | tags::skip_tag_postprocess | tags::skip_block_postprocess | tags::for_editor);
	if (!tag)
		std::cout << "failed to load tag: " << name << std::endl;
	return tag;
}

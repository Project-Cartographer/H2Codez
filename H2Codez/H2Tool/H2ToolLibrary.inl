#include "../stdafx.h"
#include "../Common/FiloInterface.h"
#include "../Common/BasicTagTypes.h"

static const char* get_h2tool_version()
{
	typedef char*(_cdecl* _get_h2tool_build_date)();
	static _get_h2tool_build_date get_h2tool_build_date = CAST_PTR(_get_h2tool_build_date, 0xEA760);

	return get_h2tool_build_date();

}
static int __cdecl TAG_LOAD(int TAG_TYPE, cstring tags_directory, int a3)
{
	typedef int(_cdecl* _TAG_LOAD)(int, cstring, int);
	static _TAG_LOAD TAG_LOAD = CAST_PTR(_TAG_LOAD, 0x533930);

	return TAG_LOAD(TAG_TYPE, tags_directory, a3);
}
static int __cdecl TAG_GET(int TAG_TYPE, int TAG_INDEX)
{
	typedef int(_cdecl* _TAG_GET)(int, int);
	static _TAG_GET TAG_GET = CAST_PTR(_TAG_GET, 0x52F150);

	return TAG_GET(TAG_TYPE, TAG_INDEX);
}
static DWORD TAG_NEW(int TAG_TYPE, char* TAG_PATH)
{
	typedef DWORD(_cdecl* _TAG_NEW)(int, char*);
	static _TAG_NEW TAG_NEW = CAST_PTR(_TAG_NEW, 0x5313F0);

	return TAG_NEW(TAG_TYPE, TAG_PATH);
}

inline static DWORD TAG_NEW(int TAG_TYPE, const char* TAG_PATH)
{
	return TAG_NEW(TAG_TYPE, const_cast<char*>(TAG_PATH));
}

inline static DWORD TAG_NEW(int TAG_TYPE, std::string TAG_PATH)
{
	return TAG_NEW(TAG_TYPE, TAG_PATH.c_str());
}

static DWORD __cdecl TAG_UNLOAD(int TAG_INDEX)
{
	typedef DWORD(_cdecl* _TAG_UNLOAD)(int);
	static _TAG_UNLOAD TAG_UNLOAD = CAST_PTR(_TAG_UNLOAD, 0x52F7C0);

	return TAG_UNLOAD(TAG_INDEX);
}
static DWORD __cdecl TAG_SAVE(int TAG_INDEX)
{
	typedef DWORD(_cdecl* _TAG_SAVE)(int);
	static _TAG_SAVE _TAG_SAVE_ = CAST_PTR(_TAG_SAVE, 0x532F40);

	return _TAG_SAVE_(TAG_INDEX);
}
static CHAR*__cdecl TAG_GET_NAME(int TAG_INDEX)
{
	typedef CHAR*(_cdecl* TAG_GET_NAME)(int);
	static TAG_GET_NAME TAG_GET_NAME_ = CAST_PTR(TAG_GET_NAME, 0x52CF50);

	return TAG_GET_NAME_(TAG_INDEX);
}
static bool __cdecl TAG_ADD_IMPORT_INFO_BLOCK(void* IMPORT_INFO_OFFSET)
{
	typedef bool(_cdecl* TAG_ADD_IMPORT_INFO_BLOCK)(void*);
	static TAG_ADD_IMPORT_INFO_BLOCK _TAG_ADD_IMPORT_INFO_BLOCK = CAST_PTR(TAG_ADD_IMPORT_INFO_BLOCK, 0x545D40);

	return _TAG_ADD_IMPORT_INFO_BLOCK(IMPORT_INFO_OFFSET);
}
static bool __cdecl TAG_ADD_IMPORT_INFO_ADD_DATA_(void* IMPORT_INFO_OFFSET, filo *FILE_REF)
{
	typedef bool(_cdecl* TAG_ADD_IMPORT_INFO_ADD_DATA_)(void*, filo*);
	static TAG_ADD_IMPORT_INFO_ADD_DATA_ _TAG_ADD_IMPORT_INFO_ADD_DATA_ = CAST_PTR(TAG_ADD_IMPORT_INFO_ADD_DATA_, 0x545E90);

	return _TAG_ADD_IMPORT_INFO_ADD_DATA_(IMPORT_INFO_OFFSET, FILE_REF);
}
static bool __cdecl TAG_FILE_CHECK_READ_ONLY_ACCESS(int TAG_INDEX, int a2)
{
	typedef bool(_cdecl* _TAG_FILE_CHECK_READ_ONLY_ACCESS)(int, int);
	static _TAG_FILE_CHECK_READ_ONLY_ACCESS TAG_FILE_CHECK_READ_ONLY_ACCESS = CAST_PTR(_TAG_FILE_CHECK_READ_ONLY_ACCESS, 0x410D20);

	return TAG_FILE_CHECK_READ_ONLY_ACCESS(TAG_INDEX, a2);
}
static void* __cdecl TAG_BLOCK_ADD_ELEMENT(void* TAG_ADDRESS)
{
	typedef void*(_cdecl* TAG_BLOCK_ADD_ELEMENT)(void*);
	static TAG_BLOCK_ADD_ELEMENT TAG_BLOCK_ADD_ELEMENT_ = CAST_PTR(TAG_BLOCK_ADD_ELEMENT, 0x533CA0);

	return TAG_BLOCK_ADD_ELEMENT_(TAG_ADDRESS);
}
static int __cdecl TAG_BLOCK_GET_ELEMENT_WITH_SIZE(void* FIELD_OFFSET, int ELEMENT_INDEX, int FIELD_SIZE)
{
	typedef int(_cdecl* TAG_BLOCK_GET_ELEMENT_WITH_SIZE)(void*, int, int);
	static TAG_BLOCK_GET_ELEMENT_WITH_SIZE TAG_BLOCK_GET_ELEMENT_WITH_SIZE_ = CAST_PTR(TAG_BLOCK_GET_ELEMENT_WITH_SIZE, 0x532970);

	return TAG_BLOCK_GET_ELEMENT_WITH_SIZE_(FIELD_OFFSET, ELEMENT_INDEX, FIELD_SIZE);
}

static int GET_STRING_ID(const char *string)
{
	typedef int (__cdecl *get_string_id)(const char *string);
	get_string_id get_string_id_impl = reinterpret_cast<get_string_id>(0x0052E830);

	return get_string_id_impl(string);
}

typedef bool (*find_tag_comparison)(void *element, void *find_data);

static unsigned int FIND_TAG_BLOCK_ELEMENT(tag_block_ref *tag_block, size_t element_size, find_tag_comparison search_func, void *data)
{
	for (int index = 0; index < tag_block->size; index++)
	{
		void *element = reinterpret_cast<void*>(TAG_BLOCK_GET_ELEMENT_WITH_SIZE(tag_block, index, element_size));
		if (search_func(element, data))
			return index;
	}
	return NONE;
}

struct string_id_cmp_info
{
	size_t offset;
	DWORD string_id;
};

static bool string_id_comparison(DWORD *element, string_id_cmp_info *find_info)
{
	DWORD current_string_id = element[find_info->offset / sizeof(DWORD)];
	return current_string_id == find_info->string_id;
}

static unsigned int FIND_TAG_BLOCK_STRING_ID(tag_block_ref *tag_block, size_t element_size, size_t offset, DWORD string_id)
{
	string_id_cmp_info search_info;
	search_info.offset = offset;
	search_info.string_id = string_id;

	return FIND_TAG_BLOCK_ELEMENT(tag_block, element_size, reinterpret_cast<find_tag_comparison>(&string_id_comparison), &search_info);
}



struct string_cmp_info
{
	size_t offset;
	const char *string;
};

static bool string_comparison(char *element, string_cmp_info *find_info)
{
	char *current_string = &element[find_info->offset];
	return !_stricmp(current_string, find_info->string);
}

static unsigned int FIND_TAG_BLOCK_STRING(tag_block_ref *tag_block, size_t element_size, size_t offset, const char *string)
{
	string_cmp_info search_info;
	search_info.offset = offset;
	search_info.string = string;

	return FIND_TAG_BLOCK_ELEMENT(tag_block, element_size, reinterpret_cast<find_tag_comparison>(&string_comparison), &search_info);
}

static unsigned int FIND_TAG_BLOCK_STRING(tag_block_ref *tag_block, size_t element_size, size_t offset, const std::string &string)
{
	return FIND_TAG_BLOCK_STRING(tag_block, element_size, offset, string.c_str());
}

static char __cdecl load_model_object_definations_(DWORD IMPORT_INFO_OFFSET_, void *proc_definations_, __int16 proc_count, filo& FILE_REF_)
{
	typedef char(_cdecl* _load_model_object_definations_)(DWORD, void *, __int16, filo&);
	static _load_model_object_definations_ load_model_object_definations_ = CAST_PTR(_load_model_object_definations_, 0x412E00);

	return load_model_object_definations_(IMPORT_INFO_OFFSET_, proc_definations_, proc_count, FILE_REF_);
}
static bool _cdecl tool_build_paths(wcstring directory, const char* Subfolder, filo& out_reference, wchar_t out_path[256], void* arg_10)
{
	typedef bool(_cdecl*_tool_build_paths)(wcstring, const char*, filo&, wchar_t[256], void*);
	static const _tool_build_paths tool_build_paths = CAST_PTR(_tool_build_paths, 0x4119B0);
	return tool_build_paths(directory, Subfolder, out_reference, out_path, arg_10);
}
static void _cdecl use_import_definitions(const void* definitions, int count, filo& reference, void* context_data, void* unk)
{
	typedef void(_cdecl* _use_import_definitions)(const void*, int, filo&, void*, void*);
	static const _use_import_definitions use_import_definitions = CAST_PTR(_use_import_definitions, 0x412100);
	use_import_definitions(definitions, count, reference, context_data, unk);
}

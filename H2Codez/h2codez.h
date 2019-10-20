#pragma once
#include "stdafx.h"
#include <assert.h> 


// Constant '\0' terminated ASCII string
typedef const char* cstring;
// '\0\0' terminated Unicode string
typedef wchar_t* wstring;
// Constant '\0\0' terminated Unicode string
typedef const wchar_t* wcstring;
typedef void(_cdecl* _tool_command_proc)(const wchar_t *argv[]);
typedef void(_cdecl* _tool_import__defination_proc)(void* FILE_REFERENCE, void* ref_ptr);
typedef char long_string[255 + 1];



#define CAST_PTR(type, ptr)		(reinterpret_cast<type>(ptr))
#define CAST_PTR_OP(type)		reinterpret_cast<type>
#define NUMBEROF_C(array) ( sizeof(array) / sizeof( (array)[0] ) )
#define NUMBEROF(array) _countof(array)
#define FLAG(bit)( 1<<(bit) )
#define TOOL_INCREASE_FACTOR 0x20
#define CHECK_FUNCTION_SUPPORT(expersion)\
	if (!expersion)                      \
		_wassert(__FUNCTIONW__ L" doesn't support this process type.", _CRT_WIDE(__FILE__), (unsigned)(__LINE__))

/* 
	Check the size of a struct at compile time, helps ensure struct is compatible with the EK.
*/
#define CHECK_STRUCT_SIZE(struct_name, size)\
	static_assert(sizeof(struct_name) == size, #struct_name " size is invalid" )

/*
	Check offset of a field in a struct at compile time, helps ensure struct is compatible with the EK.
*/
#define CHECK_STRUCT_FIELD_OFFSET(struct_name, field, offset)\
	static_assert(offsetof(struct_name, field) == offset, #struct_name " layout is invalid")

#define ASM_FUNC __declspec(naked)

/* Pads */

// Add an anonymous 8-bit (1 byte) field to a structure.
#define PAD_BYTE unsigned char : 8;
// Add an anonymous 16-bit (2 byte) field to a structure.
#define PAD_WORD unsigned short : 16;
// Add an anonymous 32-bit (4 byte) field to a structure.
#define PAD_DWORD unsigned long : 32;

/* Is this a debug build? */
bool constexpr is_debug_build()
{
#ifdef _DEBUG
	return _DEBUG;
#else
	return false;
#endif
}

enum H2EK
{
	H2Tool,
	H2Sapien,
	H2Guerilla,
	Invalid
};
class H2EK_Globals
{
	
public:
	HMODULE base;
	H2EK process_type = H2EK::Invalid;
};
extern H2EK_Globals game;

class H2Toolz
{
public:
	static bool Init();
	static void minimal_init();
private:
	static H2EK detect_type();
};

template <typename T>
inline T SwitchByMode(T tool, T sapien, T guerilla)
{
	assert(game.process_type != H2EK::Invalid);
	switch (game.process_type)
	{
	case H2Tool:
		return tool;
	case H2Sapien:
		return sapien;
	case H2Guerilla:
		return guerilla;
	}
	abort(); // this should never happen
}

inline DWORD SwitchAddessByMode(DWORD tool, DWORD sapien, DWORD guerilla)
{
	return SwitchByMode(tool, sapien, guerilla);
}

extern std::wstring_convert<std::codecvt_utf8<wchar_t>> wstring_to_string;

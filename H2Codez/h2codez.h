#include "stdafx.h"
#include <assert.h> 


// Constant '\0' terminated ascii string
typedef const char* cstring;
// '\0\0' terminated unicode string
typedef wchar_t* wstring;
// Constant '\0\0' terminated unicode string
typedef const wchar_t* wcstring;
typedef void(_cdecl* _tool_command_proc)(wcstring arguments);
typedef bool(_cdecl* _tool_dev_command_proc)(DWORD a1, DWORD TAG_INDEX);
typedef void(_cdecl* _tool_import__defination_proc)(void* FILE_REFERENCE, void* ref_ptr);
typedef char long_string[255 + 1];



#define CAST_PTR(type, ptr)		(reinterpret_cast<type>(ptr))
#define CAST_PTR_OP(type)		reinterpret_cast<type>
#define NUMBEROF_C(array) ( sizeof(array) / sizeof( (array)[0] ) )
#define NUMBEROF(array) _countof(array)
#define WIN32_FUNC(func) func
#define FLAG(bit)( 1<<(bit) )
#define BOOST_STATIC_ASSERT( ... ) static_assert(__VA_ARGS__, #__VA_ARGS__)
#define TOOL_INCREASE_FACTOR 0x20
#define INVALID_STATE(MESSAGE) _wassert(L##MESSAGE, _CRT_WIDE(__FILE__), (unsigned)(__LINE__))

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
	H2EK process_type;
	DWORD GetBase();


};
extern H2EK_Globals game;

class H2Toolz
{
public:
	static bool Init();
private:
	static H2EK detect_type();
};

inline DWORD SwitchAddessByMode(DWORD tool, DWORD sapien, DWORD guerilla)
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

//eg. {0xFF,0xEE,0xDD,0xCC} ->  {0xCC,0xDD,0xEE,0xFF}
BYTE* reverse_addr(void* address);

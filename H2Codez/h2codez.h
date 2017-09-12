#include "stdafx.h"



// Constant '\0' terminated ascii string
typedef const char* cstring;
// '\0\0' terminated unicode string
typedef wchar_t* wstring;
// Constant '\0\0' terminated unicode string
typedef const wchar_t* wcstring;
typedef void(_cdecl* _tool_command_proc)(wcstring arguments);
typedef char long_string[255 + 1];



#define CAST_PTR(type, ptr)		(reinterpret_cast<type>(ptr))
#define CAST_PTR_OP(type)		reinterpret_cast<type>
#define NUMBEROF_C(array) ( sizeof(array) / sizeof( (array)[0] ) )
#define NUMBEROF(array) _countof(array)
#define WIN32_FUNC(func) func
#define FLAG(bit)( 1<<(bit) )
#define BOOST_STATIC_ASSERT( ... ) static_assert(__VA_ARGS__, #__VA_ARGS__)

enum H2EK
{
	H2Tool,
	H2Sapien,
};
class H2EK_Globals
{
	
public:
	HMODULE base;
	H2EK process_type;
	DWORD GetBase();


};
extern H2EK_Globals game;

//as the functions says
char* wstring_to_string(char* string, int string_length, wcstring wide, int wide_length);
//eg. {0xFF,0xEE,0xDD,0xCC} ->  {0xCC,0xDD,0xEE,0xFF}
BYTE* reverse_addr(void* address);

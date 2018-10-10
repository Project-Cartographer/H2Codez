/*
    Yelo: Open Sauce SDK
		Halo 2 (Editing Kit) Edition

	See license\OpenSauce\Halo2_CheApe for specific license information
*/
#include "../stdafx.h"
#include "H2Tool_Commands.h"
#include "Animations.inl"
#include "Sounds.inl"




static void _cdecl import_model_render_proc(const wchar_t *argv[])
{
	static _tool_command_proc _import_model_render_proc = CAST_PTR(_tool_command_proc, 0x41C7A0);
	_import_model_render_proc(argv);
}



static const s_tool_command* import_class_monitor_structures = CAST_PTR(s_tool_command*, 0x97B6D8);
static const s_tool_command* import_class_monitor_bitmaps = CAST_PTR(s_tool_command*, 0x97B594);


static void _cdecl import_model_proc(wcstring* arguments)
{
	std::string object_name = wstring_to_string.to_bytes(arguments[0]);
	wcstring object_type = arguments[1];

	static wcstring object_type_names[] = {
		L"biped",
		L"vehicle",
		L"weapon",
		L"equipment",
		L"garbage",
		L"projectile",
		L"scenery",
		L"machine",
		L"control",
		L"light_fixture",
		L"sound_scenery",
		L"crate",
		L"creature",
	};
	long object_type_mask = 0;

	for(int x = 0; x < NUMBEROF(object_type_names); x++, object_type_mask = 1 << x)
		if( !_wcsicmp(object_type, object_type_names[x]))
		{
			object_type_mask = FLAG(x);
			break;
		}

	if(object_type_mask == 0)
	{
		wprintf_s(L"'%s' is not a valid object type!", object_type);
		return;
	}

	typedef void (_cdecl* _import_object_model)(cstring object_name, long object_type_mask);
	static _import_object_model import_object_model = CAST_PTR(_import_object_model, 0x4E7700);

	import_object_model(object_name.c_str(), object_type_mask);
}

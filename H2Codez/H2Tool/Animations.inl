/*
    Yelo: Open Sauce SDK
		Halo 2 (Editing Kit) Edition

	See license\OpenSauce\Halo2_CheApe for specific license information
*/

/*
spaces ' ' in the file name are replaced with ':'

allows up to 16 'var's

jma		- animation
jmm		- moving
jmt		- rotation
jmo		- overlay
jmr		- replacement
jmrx	- replacement extended
jmz		- 3d animation
jmw		- world animation
*/
#include "stdafx.h"
static void import_model_animations_proc_impl(s_file_reference& reference)
{
	static const void* animation_import_definitions = CAST_PTR(void*, 0x97DEC8);
	static cstring data_directory = "data";
	static const unsigned int sub_52A540 = 0x52A540;
	static const unsigned int sub_412430 = 0x412430;

	
	_asm {
		    push	1
			push	data_directory
			push	reference
			call	sub_52A540
			add		esp, 4 * 3

			push	0
			push	reference
			push	8
			push	animation_import_definitions
			call	sub_412430
			add		esp, 4 * 4
	}


}

/* s_intermediate_animation, 0x21C
0x0000 - char[256]
0x0100 - char[256]
0x0200 - dword, jma version
0x0204 - dword, production checksum
0x0208 - real, fps
0x020C - word, frame count // max: 0x7D0
0x0210 - dynamic_array, 0x6C, actors
	0x00 - wchar_t[32], name
	0x40 - word, node count
	0x44 - dword, node checksum
	0x48 - dynamic_array, 0xC, frames
		0x00 - dynamic_array, 0x154
			// same parsing method as actors->node frames
	0x54 - dynamic_array, 0x7C, nodes
		0x00 - wchar_t[32]
		0x40 - parent node index // 0x400A and above, if less, parse below
		0x42 - first child node index
		0x44 - next sibling node index
	0x60 - dynamic_array, 0x20, node frames
		0x00 - real, real, real
		0x0C - real, real, real, real
		0x1C - real
*/

/*
0x0000 - s_intermediate_animation

unknown_bytes[0x1E4]

0x0400 - word_flags
	FLAG(0) = 
	FLAG(1) = compression disabled
	FLAG(2) = 
	FLAG(3) = world relative
	FLAG(4) = 
	FLAG(5) = 

unknown_bytes[0x110] // enough space for a s_file_reference...

0x00514 - char[0x20000]
0x20514 - BOOL[3] // when set
	// 0 - regular message
	// 1 - warning messages
	// 2 - error messages

// I think this is the src tag
0x20520 - tag_index, jmad
0x20524 - long_flags

// I think this is the dst tag
0x20528 - tag_index, jmad
0x2052C - long_flags
	FLAG(0) = is valid?
	FLAG(1) = this and FLAG(0) set allow storing of animation message results

0x20530 - ?
0x20534 - dynamic_array, 0x88, static_frame_data related

0x20554 - dynamic_array, 0x88, static_frame_data related

0x20588 - dword, looks like a count
*/
static void _cdecl import_model_animations_proc(wcstring* arguments)
{
	typedef bool (_cdecl*_tool_build_paths)(wcstring directory,
		const char* Subfolder, s_file_reference& out_reference, wchar_t out_path[256], void* arg_10);
	static const _tool_build_paths tool_build_paths = CAST_PTR(_tool_build_paths, 0x4119B0);

	typedef void (_cdecl* _use_import_definitions)(const void* definitions, int count, 
		s_file_reference& reference, void* context_data, void*);
	static const _use_import_definitions use_import_definitions = CAST_PTR(_use_import_definitions, 0x412100);

	static const void* animation_import_definitions = CAST_PTR(void*, 0x97DEC8);


	s_file_reference reference;
	//import_model_animations_proc_impl(reference);
	
	static wchar_t out_path[256];
	if(tool_build_paths(arguments[0], "animations", reference, out_path, NULL))
	{
		use_import_definitions(animation_import_definitions, 8, reference, NULL, NULL);
	}

}
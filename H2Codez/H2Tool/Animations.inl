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
#include "../stdafx.h"
#include "../Common/FiloInterface.h"
#include "util/patches.h"
#include "Common/data/memory_dynamic_array.h"
#include "Common/TagInterface.h"
#include <type_traits>

static void import_model_animations_proc_impl(filo& reference)
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

#define KILOBYTE 0x400
#define MEGABYTE KILOBYTE * KILOBYTE

struct s_animation_compiler
{
	char field_0[1024];
	int user_flags;
	char field_404[272];
	char message_buffer[131072];
	DWORD error_caterorgy_count[3];

	struct jmad_entry {
		datum tag;
		enum _flags : int
		{
			is_valid = 1 << 0,
			has_tag = 1 << 1
		} flags = _flags::is_valid;

		void operator=(datum _tag)
		{
			tag = _tag;
			if (tag.is_valid())
				flags = static_cast<_flags>(flags | has_tag);
			else
				flags = static_cast<_flags>(flags & ~has_tag);

		}
	};
	CHECK_STRUCT_SIZE(jmad_entry, 8);

	jmad_entry source;
	jmad_entry target;
	int field_20530;
	memory_dynamic_array field_20534;
	float field_20540;
	memory_dynamic_array field_20544;
	int field_20550;
	int field_20554;
	int field_20558;
	int field_2055C;
	int field_20560;
	int field_20564;
	int field_20568;
	int field_2056C;
	int field_20570;
	int field_20574;
	int field_20578;
	int field_2057C;
	int field_20580;
	int field_20584;
	int field_20588;

	
	byte pad[KILOBYTE];
};
CHECK_STRUCT_SIZE(s_animation_compiler, 0x2058C + KILOBYTE);

s_animation_compiler animation_compiler;

static void invalid_parameter()
{
	printf("you broke strtok_s, congrates?\n");
}

static void _cdecl import_model_animations_proc(wcstring* arguments)
{
	typedef bool (_cdecl*_tool_build_paths)(wcstring directory,
		const char* Subfolder, filo& out_reference, wchar_t out_path[256], void* arg_10);
	static const _tool_build_paths tool_build_paths = CAST_PTR(_tool_build_paths, 0x4119B0);

	typedef void (_cdecl* _use_import_definitions)(const void* definitions, int count, 
		filo& reference, void* context_data, void*);
	static const _use_import_definitions use_import_definitions = CAST_PTR(_use_import_definitions, 0x412100);

	static const void* animation_import_definitions = CAST_PTR(void*, 0x97DEC8);

	PatchCall(0x74EEEC, invalid_parameter); // till someone figures out filenames
	NopFill(0x496B3B, 2); // patch version check for now

	filo reference;
	//import_model_animations_proc_impl(reference);
	
	//animation_compiler.source = tags::new_tag('jmad', "jmad_target");
	animation_compiler.target = tags::load_tag('jmad', "objects\\weapons\\melee\\gravity_hammer\\gravity_hammer", 0);
	animation_compiler.field_20534._element_size = 0x88;
	animation_compiler.field_20544._element_size = 0x88;

	static wchar_t out_path[256];
	if(tool_build_paths(arguments[0], "animations", reference, out_path, NULL))
	{
		use_import_definitions(animation_import_definitions, 8, reference, &animation_compiler, NULL);
	}

}
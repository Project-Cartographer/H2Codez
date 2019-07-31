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

#include "H2ToolLibrary.inl"
#include "Common/FiloInterface.h"
#include "Common/data/memory_dynamic_array.h"
#include "Common/TagInterface.h"
#include "util/patches.h"
#include "util/string_util.h"
#include "util/time.h"
#include "util/StringEncodingDetector.h"
#include "util/StringEndianess.h"
#include <type_traits>

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

		bool save()
		{
			return tags::save_tag(tag);
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

	// pad to stop it from corrupting other variables if we are wrong about size
	byte pad[KILOBYTE];
};
CHECK_STRUCT_SIZE(s_animation_compiler, 0x2058C + KILOBYTE);

s_animation_compiler animation_compiler;

static std::string get_jmad_path(const wchar_t *import_path)
{
	std::string import_path_utf8 = wstring_to_string.to_bytes(import_path);
	return tag_path_from_import_path(import_path_utf8);
}

static void enable_compression_printf()
{
	DWORD codec_compression_printf[] = {
		0x5AEEE5,
		0x5AEEFC,
		0x5AEF16,
		0x5AEF2E,
		0x5AEF38,
		0x5AF010
	};

	for (DWORD addr : codec_compression_printf)
		PatchCall(addr, printf);
}

wchar_t *__cdecl read_animation_file(file_reference *file, DWORD *size_out)
{
	wchar_t *output_data = nullptr;
	*size_out = NONE;
	// this is needed because the toolkit doesn't use the same allocator as us
	auto set_output_string = [&](const wchar_t *string, size_t len)
	{
		typedef wchar_t *__cdecl DEBUG_MALLOC(size_t size, char a2, char *file, int line, void *a5, void *a6, void *a7);
		auto debug_malloc = reinterpret_cast<DEBUG_MALLOC*>(0x52B540);
		*size_out = len + 1;
		output_data = debug_malloc((len + 1) * sizeof(wchar_t), 0, __FILE__, __LINE__, 0, 0, 0);
		wcsncpy_s(output_data, (len + 1), string, len);
	};

	DWORD file_size = 0;
	void *file_data = FiloInterface::read_into_memory(file, file_size);
	if (!file_data)
		return nullptr;

	size_t start_offset = 0;
	auto detected_encoding = StringEncodingDetector::detect_encoding(file_data, file_size, &start_offset);
	LOG_FUNC("encoding: %d, BOM-len: %d", detected_encoding, start_offset);

	// helper struct
	union _string_data
	{
		void *raw;
		char *narrow;
		wchar_t *wide;
	};

	_string_data data;
	data.raw = file_data;
	data.narrow += start_offset; // skip the BOM

	switch (detected_encoding)
	{
		case StringEncodingDetector::encoding::ucs16_be:
			// byteswap to native encoding
			byteswap_wide_string(data.wide, (file_size - start_offset) / 2);
			/* fall through */
		case StringEncodingDetector::encoding::ucs16_le:
			set_output_string(data.wide, (file_size - start_offset) / 2);
			break;
		case StringEncodingDetector::encoding::ascii:
		case StringEncodingDetector::encoding::utf8:
		{
			size_t string_len = file_size - start_offset;
			std::wstring string = wstring_to_string.from_bytes(data.narrow, &data.narrow[string_len]);
			set_output_string(string.c_str(), string.length());
			break;
		}
		default:
			cout << "Unknown animation file encoding" << endl;
			break;
	}

	delete[] file_data;
	return output_data;
}

static void fix_import_animations()
{
	NopFill(0x49296A, 8); // fix strtok_s breaking up
	NopFill(0x496B3B, 2); // patch version check for now
	//enable_compression_printf();
	PatchCall(0x495EE9, read_animation_file);
}

static void _cdecl import_extra_model_animations_proc(wcstring* arguments)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	fix_import_animations();

	const wchar_t *import_path = arguments[0];
	std::string jmad_path = get_jmad_path(import_path);
	std::cout << "model_animation_graph : " << jmad_path << std::endl;

	datum tag = tags::load_tag('jmad', jmad_path, tags::loading_flags::skip_post_process);
	if (!tag.is_valid()) {
		std::cout << "Unable to find tag: " << jmad_path << ".model_animation_graph" << std::endl;
		return;
	}
	
	//animation_compiler.source = tags::new_tag('jmad', "jmad_target");
	animation_compiler.target = tag;
	animation_compiler.field_20534._element_size = 0x88;
	animation_compiler.field_20544._element_size = 0x88;

	static wchar_t out_path[256];
	file_reference reference;
	std::cout << "=== finding files... ===" << std::endl;
	if (tool_build_paths(arguments[0], "animations", reference, out_path, NULL))
	{
		std::cout << "=== importing! ===" << std::endl;
		static const void* animation_import_definitions = CAST_PTR(void*, 0x97DEC8);
		use_import_definitions(animation_import_definitions, 8, reference, &animation_compiler, NULL);
	}
	std::cout << "=== saving tag! ===" << std::endl;
	animation_compiler.target.save();

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time_taken = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
	std::string time_taken_human = beautify_duration(time_taken);
	std::cout << "time taken: " << time_taken_human << std::endl;
}
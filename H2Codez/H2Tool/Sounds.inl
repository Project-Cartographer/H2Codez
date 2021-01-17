#include <intrin.h>

/*
    Yelo: Open Sauce SDK
		Halo 2 (Editing Kit) Edition

	See license\OpenSauce\Halo2_CheApe for specific license information
*/

// *.ltf - lipsync_track (unicode)
// ltf // header
// %f
// %i // count
// %i %f %f

// this is a recreation of the CE version of this function because well that's what this code expects
// all the localization and multiple directories stuff in h2 broke it
static void __cdecl filo_internal__get_absolute_path(__int16 location, char *path, char *full_path)
{
	void *caller = _ReturnAddress();
	if (location == 2 || location <= 0)
	{
		strcpy_s(full_path, 0x100, path);
	} else {
		strcpy_s(full_path, 0x100, "?:\\");
		strcat_s(full_path, 0x100, path);
	}
	LOG_FUNC("get_absolute_path: caller %p", caller);
}

static int __cdecl find_files_in_directory_hook(int flags, file_reference *main_file, int max_file_count, file_reference *files)
{
	typedef int __cdecl find_files_in_directory(int flags, file_reference* main_file, int max_file_count, file_reference* files);
	auto find_files_in_directory_impl = reinterpret_cast<find_files_in_directory*>(0x52A060);

	bool has_file_name = main_file->flags & file_reference::filename;
	// Log cause why not
	LOG_FUNC("has_file_name: %d", has_file_name);
	LOG_FUNC("path: %s", main_file->path);

	std::string directory = has_file_name ? FiloInterface::get_path_info(main_file, PATH_FLAGS::CONTAINING_DIRECTORY_PATH) : main_file->path;
	file_reference fixed_directory(directory, true);

	LOG_FUNC("directory: %s", directory.c_str());
	LOG_FUNC("new path: %s", fixed_directory.path);


	int file_count = find_files_in_directory_impl(flags, &fixed_directory, max_file_count, files);

	for (int i = 0; i < file_count; i++)
		LOG_FUNC("%d file path: %s", i, files[0].path);

	return file_count;
}


template <size_t indent = 0>
static void import_sound(file_reference *file, datum old_index = datum::null())
{
	NopFill(0x4E6B10, 5); // someone manually freed a tag....
	NopFill(0x4E691F, 5); // someone manually freed a tag again....
	void *filo_impl_get_absolute_path = reinterpret_cast<void*>(0x527EF0);

	DWORD old_code[5];
	memcpy(old_code, filo_impl_get_absolute_path, sizeof(old_code));
	WriteJmp(filo_impl_get_absolute_path, filo_internal__get_absolute_path);

	PatchCall(0x4E6827, find_files_in_directory_hook);
	PatchCall(0x4E6A64, find_files_in_directory_hook);

	static const unsigned int import_sound_impl = 0x4E6950;
	size_t message_len = indent;
	__asm
	{
		push message_len
		push file
		mov ecx, old_index
		call import_sound_impl
		add esp, 8
	}

	WriteArray(filo_impl_get_absolute_path, &old_code); // restore old filo code
}

struct sound_import_info
{
	const char *error;
	short sound_class;
	bool fit_to_adpcm;
	PAD_BYTE;
	float gain;

	enum class Compression : short
	{
		NonebigEndian = 0,
		XboxAdpcm = 1,
		ImaAdpcm = 2,
		NonelittleEndian = 3,
		Wma = 4,

		invalid = NONE,
	} target_compression;

	enum class Encoding : short
	{
		Mono = 0,
		Stereo = 1,
		Codec = 2,

		invalid = NONE,
	} encoding;

	enum class SampleRate : short
	{
		_22kHz = 0,
		_44kHz = 1,
		_32kHz = 2,

		invalid = NONE,
	} sample_rate;

	PAD_WORD;
	bool is_new_tag;
	bool tag_imported;
	char tag_name[0x100];
	PAD_WORD;
	char* src_buffer;
	char* dst_buffer;
	char* dst_buffer_2;
};
CHECK_STRUCT_SIZE(sound_import_info, 0x124);
CHECK_STRUCT_FIELD_OFFSET(sound_import_info, gain, 0x8);

static sound_import_info *import_info = reinterpret_cast<sound_import_info*>(0x97F120);

static void _cdecl import_sound_proc(wcstring* arguments)
{
/*
''         - english
'data_jpn' - japanese
'data_de'  - german
'data_fr'  - french
'data_sp'  - spanish
'data_it'  - italian
'data_kor' - korean
'data_cht' - chinese
'data_pt'  - portuguese
*/

	import_info->src_buffer = new char[0x4000000];
	import_info->dst_buffer = new char[0x4000000];
	import_info->dst_buffer_2 = new char[0x1000000];

	import_info->fit_to_adpcm = true;

	LOG_FUNC("dst: %p, src: %p", import_info->dst_buffer, import_info->src_buffer);

	file_reference file(0);
	file.add_directory("data");
	file.add_directory(utf16_to_utf8(arguments[0]));
	import_sound(&file);
	printf("Import Error: %s\n", import_info->error);

	delete[] import_info->dst_buffer_2;
	delete[] import_info->dst_buffer;
	delete[] import_info->src_buffer;
}

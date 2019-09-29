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
	printf("get_absolute_path: caller %p\n", caller);
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
	DWORD fit_to_adpcm;
	DWORD sample_rate; // enum
	short unk1;
	short unk2;
	short unk3;
	BYTE pad[2];
	BYTE is_new_tag;
	BYTE tag_imported;
	char tag_name[0x100];
};

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

	file_reference file(0);
	file.add_directory("data");
	file.add_directory(wstring_to_string.to_bytes(arguments[0]));
	import_sound(&file);
	printf("Import Error: %s\n", import_info->error);
}
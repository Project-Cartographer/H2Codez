#pragma once
#include "Tags/Bitmap.h"
#include "Common/FiloInterface.h"

/// <summary>
/// Creates a bitmap from a file on disk using GDI+
/// </summary>
/// <param name="file">The source file</param>
/// <param name="bitmap_out">Set to either the imported bitmap or nullptr</param>
/// <returns>Error message or nullptr</returns>
static const wchar_t *create_bitmap_from_other_image(_In_ file_reference* file, _Out_ bitmap_data_block** bitmap_out)
{
	*bitmap_out = nullptr;
	typedef const wchar_t* __cdecl create_bitmap_from_other_image(file_reference* a1, bitmap_data_block** a2);
	auto impl = reinterpret_cast<create_bitmap_from_other_image*>(0x4E7840);
	return impl(file, bitmap_out);
}

static void bitmap_insert_at_index(_Inout_ bitmap_block* bitmap_tag, int index, _In_ bitmap_data_block* new_bitmap) {
	typedef void __cdecl bitmap_insert_at_index(bitmap_block* bitmap_tag, int index, bitmap_data_block* new_bitmap);
	auto impl = reinterpret_cast<bitmap_insert_at_index*>(0x53A7D0);
	impl(bitmap_tag, index, new_bitmap);
}

static void bitmap_remove_by_index(_Inout_ bitmap_block* bitmap, signed int index) {
	typedef void __cdecl bitmap_remove_by_index(bitmap_block* bitmap, signed int index);
	auto impl = reinterpret_cast<bitmap_remove_by_index*>(0x53A5B0);
	impl(bitmap, index);
}

static void free_bitmap_data_block(_Inout_opt_ bitmap_data_block* bitmap) {
	if (!bitmap)
		return;
	typedef void __cdecl free_bitmap_data_block(_Inout_opt_ bitmap_data_block* bitmap);
	auto impl = reinterpret_cast<free_bitmap_data_block*>(0x71E5D0);
	impl(bitmap);
}

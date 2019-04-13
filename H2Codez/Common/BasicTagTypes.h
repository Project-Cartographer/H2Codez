#pragma once
#pragma pack(1)

#include "BlamBaseTypes.h"
#include <functional>

struct tag_ref
{
	int tag_type;
	void *tag_pointer;
	int field_8;
	datum tag_index;
};
CHECK_STRUCT_SIZE(tag_ref, 16);


template<typename T>
struct tag_block;

typedef tag_block<void> tag_block_ref;

template <typename T = void>
struct tag_block
{
	static_assert(std::is_trivially_copyable<typename T>::value || std::is_void<T>::value, "tag_block must be trivially copyable or void.");

	size_t size;
	T *data;
	void *defination;

	tag_block_ref* get_ref() {
		return reinterpret_cast<tag_block_ref*>(this);
	}
	tag_block_ref *operator&()
	{
		return get_ref();
	}

	T *operator[](size_t index)
	{
		static_assert(std::is_void<T>::value == false, "This function doesn't work for tag_block_ref");

		if (index == NONE || reinterpret_cast<size_t>(this->data) == NONE)
			return nullptr;
		if (index >= this->size)
			return nullptr;
		if (LOG_CHECK(this->data))
			return &this->data[index];
		else
			return nullptr;
	}

	T *begin()
	{
		return this->data;
	}

	T *end()
	{
		static_assert(std::is_void<T>::value == false, "This function doesn't work for tag_block_ref");
		if (this->data)
			return &this->data[this->size];
		else
			return nullptr;
	}

	size_t find_element(std::function<bool(const T*)> search_function)
	{
		static_assert(std::is_void<T>::value == false, "This function doesn't work for tag_block_ref");
		for (size_t index = 0; index < this->size; index++)
		{
			if (search_function(this->operator[](index)))
				return index;
		}
		return NONE;
	}

	void clear()
	{
		this->size = 0;
		this->data = reinterpret_cast<T*>(NONE);
	}


	// search functions

	size_t find_string_element(size_t offset, const std::string &string)
	{
		const auto find_string = [&](const T *element) -> bool {
			const char *data = reinterpret_cast<const char*>(element);
			return !_stricmp(&data[offset], string.c_str());
		};
		return find_element(find_string);
	}

	size_t find_string_id_element(size_t offset, string_id string)
	{
		auto find_string = [&](const T *element) -> bool {
			const char *data = reinterpret_cast<const char*>(element);
			const DWORD *string_id = static_cast<const DWORD*>(&data[offset]);
			return *string_id == offset.id;
		};
		return find_element(find_string);
	}
};
CHECK_STRUCT_SIZE(tag_block<void>, 12);
CHECK_STRUCT_SIZE(tag_block_ref, 12);

struct byte_ref
{
	int size;
	int stream_flags;
	int stream_offset;
	void *address;
	int definition;
};

struct g_null_block
{
};

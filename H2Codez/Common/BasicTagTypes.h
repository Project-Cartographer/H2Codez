#pragma once
#pragma pack(1)

#include "BlamBaseTypes.h"
#include "TagDefinitions.h"
#include <functional>

struct tag_reference
{
	int tag_type;
	const char *tag_name;
	int field_8;
	datum tag_index;
};
CHECK_STRUCT_SIZE(tag_reference, 16);


template<typename T>
struct tag_block;

typedef tag_block<void> tag_block_ref;

template <typename T = void>
struct tag_block
{
	static_assert(std::is_trivially_copyable<typename T>::value || std::is_void<T>::value, "tag_block must be trivially copyable or void.");

	size_t size;
	T *data;
	tag_block_defintions *defination;

	tag_block_ref* get_ref() {
		return reinterpret_cast<tag_block_ref*>(this);
	}
	tag_block_ref *operator&()
	{
		return get_ref();
	}

	bool is_valid()
	{
		if (reinterpret_cast<size_t>(this->data) == NONE)
			return false;
		if (!LOG_CHECK(this->data))
			return false;
		return true;
	}

	T *get_element(size_t index)
	{
		if (!is_valid())
			return nullptr;
		if (index == NONE || index >= this->size)
			return nullptr;
		return get_element_internal<T>(index);
	}

	T *operator[](size_t index)
	{
		return get_element(index);
	}

	T *begin()
	{
		if (is_valid())
			return get_element_internal<T>(0);
		else
			return nullptr;
	}

	T *end()
	{
		if (is_valid())
			return get_element_internal<T>(size);
		else
			return nullptr;
	}

	size_t find_element(std::function<bool(const T*)> search_function)
	{
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

private:
	template<typename T1>
	inline T1 *get_element_internal(size_t idx)
	{
		return &this->data[idx];
	}

	template<>
	inline void *get_element_internal(size_t idx)
	{
		size_t element_size = defination->latest->size;
		char *data_char = reinterpret_cast<char*>(this->data);
		return reinterpret_cast<void*>(&data_char[element_size * idx]);
	}
};
CHECK_STRUCT_SIZE(tag_block<void>, 12);
CHECK_STRUCT_SIZE(tag_block_ref, 12);

struct byte_ref
{
	int size;
	int stream_flags;
	int stream_offset;
	union {
		void *address;
		int data;
	};
	int definition;

	inline bool is_empty() const
	{
		return this->data == NONE || this->size == 0;
	}

	inline void *operator[](size_t index)
	{
		if (this->is_empty() || this->size < (int)index)
			return nullptr;
		char *data = reinterpret_cast<char*>(this->address);
		return &data[index];
	}
};
CHECK_STRUCT_SIZE(byte_ref, 20);

// special empty struct for empty tag blocks
struct g_null_block
{
};

#pragma once
#pragma pack(1)

#include "BlamBaseTypes.h"
#include "TagDefinitions.h"
#include "MemoryAllocator.h"
#include <functional>

struct tag_reference
{
	blam_tag tag_type;
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

	int32_t size;
	T *data;
	tag_block_defintions *defination;

	tag_block_ref* get_ref() {
		return reinterpret_cast<tag_block_ref*>(this);
	}
	tag_block_ref *operator&()
	{
		return get_ref();
	}

	bool is_valid() const
	{
		if (reinterpret_cast<size_t>(this->data) == NONE)
			return false;
		if (!LOG_CHECK(this->data))
			return false;
		return true;
	}

	T *get_element(size_t index) const
	{
		if (!is_valid())
			return nullptr;
		if (index == NONE || index >= static_cast<size_t>(this->size))
			return nullptr;
		return get_element_internal<T>(index);
	}

	T *operator[](size_t index) const
	{
		return get_element(index);
	}

	T *begin() const
	{
		if (is_valid())
			return get_element_internal<T>(0);
		else
			return nullptr;
	}

	T *end() const
	{
		if (is_valid())
			return get_element_internal<T>(size);
		else
			return nullptr;
	}

	int32_t find_element(std::function<bool(const T*)> search_function) const
	{
		for (int32_t index = 0; index < this->size; index++)
		{
			if (search_function(this->operator[](index)))
				return index;
		}
		return NONE;
	}

	void clear()
	{
		if (is_valid())
			HEK_DEBUG_FREE(this->data);
		this->size = 0;
		this->data = reinterpret_cast<T*>(NONE);
	}

	// search functions

	int32_t find_string_element(size_t offset, const char *string) const
	{
		const auto find_string = [&](const T *element) -> bool {
			const char *data = reinterpret_cast<const char*>(element);
			return !_stricmp(&data[offset], string);
		};
		return find_element(find_string);
	}

	inline int32_t find_string_element(size_t offset, const std::string &string) const
	{
		return find_string_element(offset, string.c_str());
	}

	int32_t find_string_id_element(size_t offset, string_id string) const
	{
		auto find_string = [&](const T *element) -> bool {
			const uint8_t *data = reinterpret_cast<const uint8_t*>(element);
			const uint32_t *string_id = reinterpret_cast<const uint32_t*>(&data[offset]);
			return *string_id == string.get_packed();
		};
		return find_element(find_string);
	}

	inline int32_t find_string_id_element(size_t offset, const char *string) const
	{
		return find_string_id_element(offset, string_id::find_by_name(string));
	}

	inline int32_t find_string_id_element(size_t offset, const std::string &string) const
	{
		return find_string_id_element(offset, string.c_str());
	}

private:
	template<typename T1>
	inline T1 *get_element_internal(size_t idx) const
	{
		return &this->data[idx];
	}

	template<>
	inline void *get_element_internal(size_t idx) const
	{
		size_t element_size = defination->latest->size;
		uint8_t *data_char = reinterpret_cast<uint8_t*>(this->data);
		return &data_char[element_size * idx];
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

	struct tag_data_definition
	{
		const char* name;
		int flags;
		int alignment_bit;
		int maximum_size;
		const char* max_size_string;
		void* byte_swap_proc;
		void* copy_proc;
	} *definition;

	inline bool is_empty() const
	{
		ASSERT_CHECK(this);
		return this->data == NONE || this->size == 0;
	}

	inline void *operator[](size_t index)
	{
		ASSERT_CHECK(this->is_valid());
		if (LOG_CHECK(index > INT_MAX) || this->is_empty() || this->size < (int)index)
			return nullptr;
		char *data = reinterpret_cast<char*>(this->get_data());
		return &data[index];
	}

	// resize, returns success
	inline bool resize(int new_size) {
		ASSERT_CHECK(this->is_valid());
		if (new_size < 0 || new_size > this->definition->maximum_size)
			return false;
		return this->realloc(new_size);
	}

	// check if structure is valid
	inline bool is_valid() const {
		if (this == nullptr)
			return false;
		if (this->address == nullptr)
			return false;
		if (this->definition == nullptr)
			return false;
		if (is_debug_build()) {
			if (IsBadReadPtr(this->definition, sizeof(tag_data_definition)))
				return false;
			if (this->data != NONE && IsBadWritePtr(this->address, this->size))
				return false;
		}
		return true;
	}

	inline const char *c_str() {
		return reinterpret_cast<char*>(get_data());
	}

private:

	inline void *get_data() const
	{
		return this->is_empty() ? nullptr : this->address;
	}

	inline bool realloc(int new_size)
	{
		auto data = HEK_DEBUG_REALLOC(this->get_data(), new_size, this->definition->alignment_bit, "tag", "data", this->definition->name);
		if (data) {
			this->size = new_size;
			this->address = data;
			return true;
		} else if (new_size == 0) {
			this->size = 0;
			this->data = -1;
			return true;
		}
		return false;
	}
};
CHECK_STRUCT_SIZE(byte_ref, 20);

// special empty struct for empty tag blocks
struct g_null_block
{
};

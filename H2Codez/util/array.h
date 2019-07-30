#pragma once
#include <array>

namespace array_util
{
	// does the array contain a key?
	template <typename T, size_t size> bool contains(const T(&array)[size], T item)
	{
		for (T element : array)
		{
			if (element == item)
				return true;
		}
		return false;
	}

	// index of the largest value?
	template <typename T, size_t size> size_t get_index_of_largest(const T(&_array)[size])
	{
		size_t largest_idx = 0;
		auto *largest_value = &_array[0];

		for (size_t idx = 0; idx < size; idx++)
		{
			if (_array[idx] > *largest_value)
			{
				largest_value = &_array[idx];
				largest_idx = idx;
			}
		}
		return largest_idx;
	}
}

template<size_t _count, typename t1, typename t2>
class static_map
{
public:
	std::array<std::pair<t1, t2>, _count> _data;

	constexpr size_t size() const
	{
		return _count;
	}

	constexpr size_t max_size() const
	{
		return _count;
	}

	constexpr size_t empty() const
	{
		return _count == 0;
	}

	constexpr size_t get_invalid_idx() const
	{
		return -1;
	}


	size_t find_key_idx(const t1& key) const
	{
		for (size_t i = 0; i < size(); i++)
		{
			if (_data[i].first == key)
				return i;
		}
		return get_invalid_idx();
	}

	const t2& at(const t1& key) const
	{
		size_t key_idx = find_key_idx(key);
		if (key_idx == get_invalid_idx())
			throw std::out_of_range("No such key");
		return _data[key_idx].second;
	}

	const t2& operator[](const t1& key) const
	{
		return at(key);
	}

	size_t count(const t1& key) const
	{
		size_t key_idx = find_key_idx(key);
		if (key_idx != get_invalid_idx())
			return 1;
		else
			return 0;
	}
};

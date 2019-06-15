#pragma once

namespace array_util
{
	template <typename T, size_t size> bool contains(const T(&array)[size], T item)
	{
		for (T element : array)
		{
			if (element == item)
				return true;
		}
		return false;
	}
}

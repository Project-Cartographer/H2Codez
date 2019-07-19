#pragma once
struct memory_dynamic_array
{
	size_t _element_size = 0;
	size_t _count = 0;
	void*  _data = nullptr;
};

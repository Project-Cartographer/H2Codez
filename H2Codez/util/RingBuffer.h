#pragma once
#include <vector>
#include "numerical.h"

template <typename Type>
class RingBuffer
{
public:
	RingBuffer(size_t max_size) :
		_max_size(max_size)
	{
		if (max_size < 1)
			throw std::length_error("Bad size");
		_data.reserve(max_size);
	}
	RingBuffer() = delete;

	/*
		Returns buffer size
	*/
	size_t size() noexcept
	{
		return _full ? _max_size : _next_index;
	}

	/*
		Checks if buffer is empty
	*/
	bool empty() noexcept
	{
		return size() == 0;
	}


	/*
		Get element at relative offset, returns default value on failure
	*/
	Type get(int offset)
	{
		if (empty())
			return Type();
		return _data[relative_offset_to_abs(offset)];
	}

	/*
		Get element at relative offset, returns success
	*/
	bool get(int offset, Type &value) noexcept
	{
		if (empty())
			return false;
		value = _data[relative_offset_to_abs(offset)];
		return true;
	}

	/*
		Get element at relative offset, returns success
	*/
	bool get(int offset, Type *value) noexcept
	{
		if (value == nullptr)
			return false;
		if (empty())
			return false;
		*value = _data[relative_offset_to_abs(offset)];
		return true;
	}

	/*
		Add an eletement at current index
	*/
	void push(const Type &value)
	{
		_data.insert(_data.begin() + _next_index, value);
		_next_index = (_next_index + 1) % _max_size;
		if (_next_index == 0)
			_full = true;
	}

	/*
		Clears the buffer
	*/
	void clear() noexcept
	{
		_data.clear();
		_next_index = 0;
		_full = false;
	}

	/*
		Resize the buffer, must be greater than one
	*/
	void resize(size_t size, const Type& default_val = Type())
	{
		if (size < 1)
			throw std::length_error("Bad size");
		if (size == _max_size)
			return;
		_data.resize(size, default_val);
		if (size > _max_size)
		{
			_full = false;
		} else {
			_next_index = min(_next_index, size - 1);
			if (_next_index == (size - 1))
				_full = true;
		}
		_max_size = size;
	}

private:
	size_t relative_offset_to_abs(int rel_offset) noexcept
	{
		return numerical::real_modulo<int>((get_previous_index() + rel_offset), size());
	}

	size_t get_previous_index()
	{
		if (_next_index != 0)
			return _next_index - 1;
		else
			return _max_size - 1;
	}

	std::vector<Type> _data;
	size_t _max_size;
	size_t _next_index = 0;
	bool _full = false;
};
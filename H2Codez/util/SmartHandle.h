#pragma once
#include "stdafx.h"

/*
	Doesn't fully work for processes because GetCurrentProcess returns INVALID_HANDLE_VALUE
*/
class SmartHandle
{
public:
	// disable copy
	SmartHandle(const SmartHandle &) = delete;
	SmartHandle & operator= (const SmartHandle &) = delete;
	// allow move
	SmartHandle(SmartHandle &&) = default;
	SmartHandle &  operator= (SmartHandle &&) = default;

	SmartHandle(HANDLE handle) :
		_handle(handle)
	{};

	/*
		Attempts to close the handle and returns success
	*/
	bool close()
	{
		if (!is_valid())
			return false;
		bool success = LOG_CHECK(CloseHandle(_handle));
		if (success)
			_handle = INVALID_HANDLE_VALUE;
		return success;
	}

	/* Is handle valid? */
	bool is_valid() const { return _handle != INVALID_HANDLE_VALUE && _handle != NULL; }

	HANDLE get() { return  _handle; }

	~SmartHandle()
	{
		close();
	}

private:
	HANDLE _handle = INVALID_HANDLE_VALUE;
};
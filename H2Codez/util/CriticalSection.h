#pragma once
#include "stdafx.h"

class CriticalSection
{
public:
	inline CriticalSection(size_t spin_count = 0)
	{
		InitializeCriticalSectionAndSpinCount(&_section, spin_count);
	}
	inline ~CriticalSection()
	{
		DeleteCriticalSection(&_section);
	}

	inline void enter()
	{
		EnterCriticalSection(&_section);
	}

	inline bool try_enter()
	{
		return TryEnterCriticalSection(&_section);
	}

	inline void leave()
	{
		LeaveCriticalSection(&_section);
	}
	CriticalSection(const CriticalSection &) = delete;
	CriticalSection(CriticalSection&& boo) = delete;

private:
	CRITICAL_SECTION _section;
};

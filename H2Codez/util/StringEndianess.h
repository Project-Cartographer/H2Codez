#pragma once
#include <stdlib.h>
#include <string.h>


inline void byteswap_wide_string(wchar_t *string, size_t count)
{
	unsigned short *data = reinterpret_cast<unsigned short *>(string);
#pragma loop(hint_parallel(0))
	for (size_t i = 0; i < count; i++)
		data[i] = _byteswap_ushort(data[i]);
}

inline void byteswap_wide_string(wchar_t *string)
{
	byteswap_wide_string(string, wcslen(string) + 1);
}

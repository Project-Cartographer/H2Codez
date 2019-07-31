#pragma once
#include "h2codez.h"
#include "numerical.h"
#include "array.h"
#include <map>

/*
	So this all assumes we are dealing with English text
*/
namespace StringEncodingDetector
{
	/*
		heuristic constants
	*/

	static constexpr float wchar_confidence_required = .65f;
	static constexpr float unknown_endianness_warning_threshold = .08f;

	enum class endianness : unsigned int
	{
		little,
		big,
		unknown,

		count
	};

	enum class encoding : unsigned int
	{
		ascii,
		utf8,
		ucs16_le,
		ucs16_be,
		unknown, // bad news

		count
	};

#pragma region character heuristics

	// check if the character is likely to be a wchar_t
	inline bool is_character_ucs16(const wchar_t c)
	{
		return (LOBYTE(c) == 0) || (HIBYTE(c) == 0);
	}

	// detect likely endianness of a wchar_t
	inline endianness detect_character_endianess(const wchar_t c)
	{
		if (HIBYTE(c) == 0 && LOBYTE(c) == 0)
			return endianness::unknown;
		if (HIBYTE(c) == 0)
			return endianness::little;
		if (LOBYTE(c) == 0)
			return endianness::big;
		return endianness::unknown;
	}

	// is pure ASCII encoding (7-bit)
	inline bool is_ascii(char c)
	{
		return c >= 0 && c <= 127;
	}

#pragma endregion

	// returns if the string is pure ASCII
	inline bool is_string_ascii(const char *string, size_t size)
	{
		for (size_t idx = 0; idx < size; idx++)
		{
			if (!is_ascii(string[idx]))
				return false;
		}
		return true;
	}

	// returns true if string is likely to be English USC-16
	inline bool is_string_likely_usc16(const wchar_t *string, size_t size)
	{
		size_t detected_count = 0;
		for (size_t idx = 0; idx < size; idx++)
			detected_count += is_character_ucs16(string[idx]) ? 1 : 0;
		return numerical::div<float>(detected_count, size) > wchar_confidence_required;
	}

	// returns what endianness is more likely
	inline endianness detect_usc16_endianness(const wchar_t *string, size_t size)
	{
		size_t counts[(int)endianness::count] = {};

		for (size_t idx = 0; idx < size; idx++)
			counts[(int)detect_character_endianess(string[idx])]++;

		float unknown_percentage = numerical::div<float>(counts[(int)endianness::unknown], size);
		if (unknown_percentage > unknown_endianness_warning_threshold)
		{
			LOG_FUNC("Warning high number of characters of unknown endianness %f", unknown_percentage);
		}

		return static_cast<endianness>(array_util::get_index_of_largest(counts));
	}

	constexpr static unsigned char utf8_BOM[] = { 0xEF, 0xBB, 0xBF };

	// detect BOM if any, will optionally set data_start to the offset of the actual data (end of BOM)
	inline encoding detect_BOM(const unsigned char *data, size_t *data_start = nullptr)
	{
		auto set_start = [&](size_t offset)
		{
			if (data_start)
				*data_start = offset;
		};

		if (memcmp(data, utf8_BOM, sizeof(utf8_BOM)) == 0)
		{
			set_start(sizeof(utf8_BOM));
			return encoding::utf8;
		}
		if (data[0] == 0xFE && data[1] == 0xFF)
		{
			set_start(2);
			return encoding::ucs16_be;
		}
		if (data[0] == 0xFF && data[1] == 0xFE)
		{
			set_start(2);
			return encoding::ucs16_le;
		}

		set_start(0);
		return encoding::unknown;
	}

	inline encoding detect_encoding(const void *data, size_t data_size, size_t *data_start = nullptr)
	{
		union _data
		{
			const unsigned char *byte;
			const char *narrow;
			const wchar_t *wide;
		};

		_data char_data;
		char_data.wide = reinterpret_cast<const wchar_t *>(data);

		if (data_size < 4) // too small to tell
			return encoding::unknown;
		auto bom_type = detect_BOM(char_data.byte, data_start);
		if (bom_type != encoding::unknown)
			return bom_type;
		// no BOM time for heuristics...

		LOG_FUNC("Using heuristics");
		// Data size needs to be a multiple of 2 for UCS-16
		if (data_size % 2 == 0)
		{
			if (is_string_likely_usc16(char_data.wide, data_size / 2))
			{
				auto byte_order = detect_usc16_endianness(char_data.wide, data_size / 2);
				if (LOG_CHECK(byte_order != endianness::unknown))
					return byte_order == endianness::big ? encoding::ucs16_be : encoding::ucs16_le;
				LOG_FUNC("Unable to detect endianness...");
				return encoding::unknown;
			}
		}

		// just assume it's UTF-8 if it's not ASCII (good enough for our use-case)
		return is_string_ascii(char_data.narrow, data_size) ? encoding::ascii : encoding::utf8;
	}
}

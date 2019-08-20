#pragma once
#include "Common\BlamBaseTypes.h"

class hs_ai_type
{
public:
	inline hs_ai_type()
	{
	};

	inline hs_ai_type(uint32_t packed_value)
	{
		_type = static_cast<type>(packed_value >> 30);
		switch (_type)
		{
			case _type_squad:
			case _type_squad_group:
				squad = packed_value & 0xFFFF;
				break;
			case _type_starting_location:
				starting_location.location = packed_value & 0xFFFF;
				starting_location.squad = (packed_value >> 16) & 0x3FFF;
				break;
			default:
				LOG_FUNC("UNIMPLEMENTED");
				break;
		}
	}

	enum type
	{
		_type_squad,
		_type_squad_group,
		_type_unknown,
		_type_starting_location,

		_type_none = NONE
	};

	inline uint32_t get_packed() const
	{
		uint32_t packed = _type << 30;
		switch (_type)
		{
			case _type_squad:
			case _type_squad_group:
				packed |= squad;
				break;
			case _type_starting_location:
				packed |= starting_location.location;
				packed |= starting_location.squad << 16;
				break;
			default:
				LOG_FUNC("UNIMPLEMENTED");
				return NONE;
		}
		return packed;
	}

	inline type get_type() const { return _type; }
	inline bool is_type_set() const { return  get_type() != _type_none; }

	inline void set_squad(uint32_t _squad)
	{
		ASSERT_CHECK(_squad <= 0xFFFF);
		_type = _type_squad;
		squad = _squad;
	}

	inline void set_squad_group(uint32_t _squad_group)
	{
		ASSERT_CHECK(_squad_group <= 0xFFFF);
		_type = _type_squad_group;
		squad_group = _squad_group;
	}

	inline void set_starting_location(uint32_t _squad, uint32_t _location)
	{
		ASSERT_CHECK(_location <= 0xFFFF);
		ASSERT_CHECK(_squad <= 0x3FFF);
		_type = _type_starting_location;
		starting_location.squad = _squad;
		starting_location.location = _location;
	}

private:

	type _type = _type_none;
	union {
		uint32_t squad;
		uint32_t squad_group;
		struct {
			uint32_t location = NONE;
			uint32_t squad = NONE;
		} starting_location;
	};
};

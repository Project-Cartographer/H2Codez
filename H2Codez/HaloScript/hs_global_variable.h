#pragma once
#pragma pack(1)

#include "hs_types.h"

namespace HaloScriptCommon
{
	/* Built in global variable */
	struct hs_global_variable {
		const char *name;
		hs_type type;
		WORD pad;
		void *variable_ptr;

		constexpr hs_global_variable
		(
			const char *_name,
			hs_type _type,
			void *_variable_ptr = nullptr
		) :
			name(_name),
			type(_type),
			pad(0),
			variable_ptr(_variable_ptr)
		{
		}
	};
	static_assert(sizeof(hs_global_variable) == 0xC, "Bad struct size");

	// global variable datum entry
	struct s_script_global
	{
		// datum header
		WORD datum_salted_idx;
		WORD pad;

		DWORD value;
	};
	CHECK_STRUCT_SIZE(s_script_global, 0x8);
}
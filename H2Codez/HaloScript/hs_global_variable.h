#pragma once
#pragma pack(1)

#include "hs_types.h"

namespace HaloScriptCommon
{
	struct hs_global_variable {
		const char *name;
		hs_type type;
		void *variable_ptr;

		constexpr hs_global_variable
		(
			const char *_name,
			hs_type _type,
			void *_variable_ptr = nullptr
		) :
			name(_name),
			type(_type),
			variable_ptr(_variable_ptr)
		{
		}
	};
	static_assert(sizeof(hs_global_variable) == 0xC, "Bad struct size");
}
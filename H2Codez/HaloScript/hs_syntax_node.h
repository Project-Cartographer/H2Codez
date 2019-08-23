#pragma once
#include "hs_types.h"
#include "hs_opcodes.h"
#include "Common/BlamBaseTypes.h"
#include "h2codez.h"

namespace HaloScriptCommon
{
	struct hs_script_node
	{
		WORD datum_header;
		union {
			WORD hs_constant_type;
			WORD hs_script_idx;
			hs_opcode hs_function_idx;
			hs_type constant_type;
		};
		hs_type value_type;
		enum _flags : WORD
		{
			primitive = 1,
			user_function = 2,
			global_reference = 4

		} flags;
		datum next_node;
		DWORD string_value_offset;
		DWORD value;

		/* Is value the index of a global variable */
		inline bool is_global_variable_ref() const
		{
			return flags & global_reference;
		}
	};
	CHECK_STRUCT_SIZE(hs_script_node, 0x14);
}

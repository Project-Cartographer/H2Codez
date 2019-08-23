#pragma once
#include "h2codez.h"
#include "HaloScript\hs_syntax_node.h"
#include "Common\data\data_array.h"

namespace HaloScriptCommon
{
	inline s_data_array *hs_get_script_nodes()
	{
		auto script_nodes = reinterpret_cast<s_data_array**>(SwitchAddessByMode(0x00BCBF4C, 0x00A9CC14, 0));
		CHECK_FUNCTION_SUPPORT(script_nodes);
		return *script_nodes;
	}

	inline hs_script_node *hs_get_script_node(uint16_t index)
	{
		return hs_get_script_nodes()->datum_get<hs_script_node>(index);
	}

	/*
		get haloscript string data
	*/
	inline const char *hs_get_string_data()
	{
		auto hs_string_data = reinterpret_cast<const char **>(SwitchAddessByMode(0x00CDB198, 0x00B21BF8, 0));
		CHECK_FUNCTION_SUPPORT(hs_string_data);
		return *hs_string_data;
	}

	/*
		get string data at offset
	*/
	inline const char *hs_get_string_data(uint32_t offset)
	{
		return &hs_get_string_data()[offset];
	}

	/*
		get string data for syntax node
	*/
	inline const char *hs_get_string_data(hs_script_node *syntax_node)
	{
		return hs_get_string_data(syntax_node->string_value_offset);
	}

	struct hs_error_info
	{
		const char *message;
		uint32_t offset;
	};
	CHECK_STRUCT_SIZE(hs_error_info, 8);

	inline hs_error_info *get_hs_error_info()
	{
		return reinterpret_cast<hs_error_info*>(SwitchAddessByMode(0x00CDB1AC, 0x00B21C0C, 0));
	}

	// helper function for reporting an error parsing a syntax element
	inline void hs_parser_error(hs_script_node *script_node, const char *error)
	{
		static char hs_error[0x1024];
		hs_error_info *error_info = get_hs_error_info();
		CHECK_FUNCTION_SUPPORT(error_info);

		strncpy(hs_error, error, sizeof(hs_error));

		error_info->message = hs_error;
		error_info->offset = script_node->string_value_offset;
		script_node->value = NONE;
	}

	// helper function for reporting an error parsing a syntax element
	inline void hs_parser_error(hs_script_node *script_node, const std::string &error)
	{
		hs_parser_error(script_node, error.c_str());
	}

	inline void hs_parser_errorf(hs_script_node *script_node, const char *format, ...)
	{
		char hs_error[0x1024];
		va_list argptr;

		va_start(argptr, format);

		vsprintf_s(hs_error, format, argptr);
		hs_parser_error(script_node, hs_error);

		va_end(argptr);
	}

	inline void **hs_get_type_parser_table()
	{
		return reinterpret_cast<void**>(SwitchAddessByMode(0x009F0C88, 0x009EDB38, 0));
	}
}

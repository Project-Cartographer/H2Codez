#pragma once
#pragma pack(1)

#include "hs_types.h"
#include "../stdafx.h"
#include "../Common/BasicTagTypes.h"

namespace HaloScriptCommon
{
	typedef char(__cdecl *func_check)(__int16 opcode, datum thread_id);
	typedef void (__cdecl *func_impl)(__int16 opcode, datum thread_id, char user_cmd);

	struct hs_command
	{
		hs_type return_type;
		const char *name;
		DWORD unk1 = 0;
		func_check check_command_args;
		func_impl command_impl;
		const char *desc;
		const char *usage;
		WORD arg_count = 0;
		WORD arg_type_array[];

		hs_command(char *cmd_name,
			hs_type ret_type,
			func_check arg_check,
			func_impl impl,
			const char *_desc = nullptr,
			const char *_usage = nullptr) :
			name(cmd_name),
			return_type(ret_type),
			check_command_args(arg_check),
			command_impl(impl),
			desc(_desc),
			usage(_usage)
		{
		}

	};
	CHECK_STRUCT_SIZE(hs_command, 0x1E);

	inline hs_command *NewCommand(const char *cmd_name,
		hs_type ret_type,
		func_check arg_check,
		func_impl impl,
		const char *_desc = nullptr,
		const char *_usage = nullptr,
		int _arg_count = 0,
		const hs_type *arg_types = nullptr)
	{
		size_t cmd_alloc_size = sizeof(hs_command) + ((_arg_count + 1) * sizeof(WORD));
		hs_command *cmd = static_cast<hs_command*>(malloc(cmd_alloc_size));
		memset(cmd, 0, cmd_alloc_size);
		cmd->name = cmd_name;
		cmd->return_type = ret_type;
		cmd->check_command_args = arg_check;
		cmd->command_impl = impl;
		cmd->desc = _desc;
		cmd->usage = _usage;
		cmd->arg_count = _arg_count;
		if (arg_types && _arg_count > 0)
		{
			for (int i = 0; i < _arg_count; i++)
				cmd->arg_type_array[i] = static_cast<WORD>(arg_types[i]);
		}
		return cmd;
	}
}
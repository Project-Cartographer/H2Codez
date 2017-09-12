#include "stdafx.h"
#include "ToolCommandDefinitions.inl"
#include "H2Tool_extra_commands.inl"
#include "Patches.h"


//I should better leave  the H2tool version i am using
//tool  debug pc 11081.07.04.30.0934.main  Apr 30 2007 09:37:08


static const s_tool_command* h2tool_extra_commands[] = {
	&import_model_render,
	&import_model_collision,
	&import_model_physics,
	&import_model,
	&import_model_animations,
	import_class_monitor_structures,
	import_class_monitor_bitmaps,
	&import_sound,
	&tool_build_structure_from_jms,
	&h2dev_extra_commands_defination,

};

int __cdecl s_tool_command_compare(void *, const void* lhs, const void* rhs)
{
	const s_tool_command* _lhs = *CAST_PTR(const s_tool_command* const*, lhs);
	const s_tool_command* _rhs = *CAST_PTR(const s_tool_command* const*, rhs);

	const wchar_t* l = _lhs->name;
	const wchar_t* r = _rhs->name;
	if (l[0] == L'~') l++;
	if (r[0] == L'~') r++;

	return _wcsicmp(l, r);
}

void _H2ToolAttachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());


	//  DetourAttach(&(PVOID&)pPlayerSpeed,OnPlayerSpeed);	

	//  DetourAttach(&(PVOID&)pcreatechar,h_CreateCharacter);
	//DetourAttach(&(PVOID&)pXYZ, h_XYZUpdate);


	DetourTransactionCommit();
	return;
}
void _H2ToolDetachHooks()
{
	//DetourDetach(&(PVOID&)pgameoptions,h_GetGameOptions);
	// DetourDetach(&(PVOID&)pMapSelect,h_MPMapSelect);
	// DetourDetach(&(PVOID&)psquadsettings,h_SquadSettings);	
	return;
}

void H2Tool_Extras::Increase_structure_size_Check()
{

	BYTE Size_Patch[4] = { 0xF2,0xF2,0xF2,0x0F };

	WriteBytesASM(0x41F83A, Size_Patch, 4);

}

void H2Tool_Extras::Enable_sp_map_compiling()
{

}

void H2Tool_Extras::Initialize()
{
	H2PCTool.WriteLog("Dll Successfully Injected to H2Tool");

	this->Increase_structure_size_Check();
	this->AddExtraCommands();

}

void H2Tool_Extras::AddExtraCommands()
{
	H2PCTool.WriteLog("Adding Extra Commands to H2Tool");
#pragma region New Function Defination Creation 

	BYTE k_number_of_tool_commands = 0xC;
	BYTE k_number_of_tool_commands_new = 0xC + NUMBEROF(h2tool_extra_commands);

	// Tool's original tool commands
	static const s_tool_command* const* tool_import_classes = CAST_PTR(s_tool_command**, 0x97B6EC);
	// The new tool commands list which is made up of tool's
	// and [yelo_extra_tool_commands]
	static s_tool_command* tool_commands[0xC + NUMBEROF(h2tool_extra_commands)];

	// copy official tool commands
	memcpy_s(tool_commands, sizeof(tool_commands),
		tool_import_classes, sizeof(s_tool_command*) * k_number_of_tool_commands);
	// copy yelo tool commands
	memcpy_s(&tool_commands[k_number_of_tool_commands], sizeof(h2tool_extra_commands),
		h2tool_extra_commands, sizeof(h2tool_extra_commands));

	// Now I know my ABCs
	qsort_s(tool_commands, NUMBEROF(tool_commands), sizeof(s_tool_command*), s_tool_command_compare, NULL);
#pragma endregion
#pragma region UpdateTool_function References

	static s_tool_command*** tool_commands_references[] = {
		CAST_PTR(s_tool_command***, 0x410596),
		CAST_PTR(s_tool_command***, 0x41060E),
		CAST_PTR(s_tool_command***, 0x412D86),
	};
	static BYTE* tool_commands_count[] = {
		CAST_PTR(BYTE*, 0x4105E5),
		CAST_PTR(BYTE*, 0x412D99),
	};

	// update references to the tool command definitions

	DWORD new_tool_defination_Table_address = (DWORD)&tool_commands[0];

	H2PCTool.WriteLog("New Tool_Definations Addy : %0X", new_tool_defination_Table_address);
	
	DWORD c = new_tool_defination_Table_address;
	BYTE *f = (BYTE*)(&c);
	BYTE Reference_Patch[4] = { f[0],f[1],f[2],f[3] };//move to ecx		


	H2PCTool.WriteLog("Reversed : %0X %0X %0X %0X", f[0], f[1], f[2], f[3]);

	for (int x = 0; x < NUMBEROF(tool_commands_references); x++)	WriteBytesASM((DWORD)tool_commands_references[x], Reference_Patch, 4); 
	


#pragma endregion
#pragma region UpdateTool_functionCount References
	// update code which contain the tool command definitions count



	BYTE Count_Patch[1] = { k_number_of_tool_commands_new }; // cmp     si, 0Ch 


	for (int x = 0; x < NUMBEROF(tool_commands_count); x++)			WriteBytesASM((DWORD)tool_commands_count[x], Count_Patch, 1);





#pragma endregion
}
#include "stdafx.h"
#include "H2Guerilla.h"
#include "Patches.h"
#include "resource.h"

typedef int(__fastcall *toggle_expert_mode)(int thisptr, int __unused);
toggle_expert_mode toggle_expert_mode_orginal;

typedef HMENU (WINAPI *LoadMenuTypedef)(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName);
LoadMenuTypedef LoadMenuOrginal;

typedef void(__fastcall *CCmdUI__Enable)(void *thisptr, BYTE _, int a2);
CCmdUI__Enable CCmdUI__Enable_Orginal;

typedef int (__fastcall *CCmdTarget__OnCmdMsg)(void *thisptr, BYTE _, unsigned int msg, void *a3, void *a4, void *AFX_CMDHANDLERINFO);
CCmdTarget__OnCmdMsg CCmdTarget__OnCmdMsg_Orginal;

HMENU main_menu;
bool enable_advanced_shaders = true;

int __fastcall CCmdTarget__OnCmdMsg_hook(void *thisptr, BYTE _, unsigned int msg, void *a3, void *a4, void *AFX_CMDHANDLERINFO)
{
	CheckMenuItem(main_menu, ID_EDIT_ADVANCEDSHADERVIEW, enable_advanced_shaders ? MF_CHECKED : MF_UNCHECKED);
	if (msg == ID_EDIT_ADVANCEDSHADERVIEW && !AFX_CMDHANDLERINFO && !a3 && !a4) {
		H2GuerrilaPatches::toggle_display_templates();
		return true;
	}
	return CCmdTarget__OnCmdMsg_Orginal(thisptr, 0, msg, a3, a4, AFX_CMDHANDLERINFO);
}

void __fastcall CCmdUI__Enable_Hook(void *thisptr, BYTE _, int a2)
{
	CCmdUI__Enable_Orginal(thisptr, 0, a2);
	EnableMenuItem(main_menu, ID_EDIT_ADVANCEDSHADERVIEW, MF_ENABLED);
}


HMENU WINAPI LoadMenuHook(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName)
{
	int menu_id = reinterpret_cast<int>(lpMenuName);
	if (menu_id == 1000 || menu_id == 6014) {
		return main_menu;
	}
	return LoadMenuOrginal(hInstance, lpMenuName);
}

int __fastcall toggle_expert_mode_hook(int thisptr, int __unused)
{
	int return_value = toggle_expert_mode_orginal(thisptr, 0);
	H2GuerrilaPatches::update_field_display();
	return return_value;
}

void H2GuerrilaPatches::update_field_display()
{
	char *expert_mode = CAST_PTR(char*,0x9AF809);
	if (*expert_mode) {
		// Display All Fields
		NopFill(0x44CDAA, 0x8);
	}
	else {
		BYTE orginal_code[8] = { 0x84, 0xC0, // test al, al
			0x0F, 0x84, 0x72, 0x01, 0x00, 0x00 }; // jz loc_44CF24
		WriteBytesASM(0x44CDAA, orginal_code, 8);
	}
}

void H2GuerrilaPatches::toggle_display_templates()
{
	enable_advanced_shaders = !enable_advanced_shaders;
	if (enable_advanced_shaders) {
		// Disable Tag Template Views
		BYTE patch[0x3] = { 0x33, 0xC0, 0xC3 }; // xor eax, eax; retn
		WriteBytesASM(0x48E730, patch, 0x3);
	} else {
		// orignal code
		BYTE patch[0x3] = { 0x8B, 0x4C, 0x24 };
		WriteBytesASM(0x48E730, patch, 0x3);
	}
}

void H2GuerrilaPatches::Init()
{
#pragma region Patches
	// Start patches copied from opensauce

	// Allow Base Tag Group Creation
	NopFill(0x409FE0, 11);

	// Display all Tags
	NopFill(0x409FEE, 0x23);
	NopFill(0x4476E7, 0x23);

	BYTE patch_can_display_tag[0x3] = { 0xB0, 0x01, 0xC3 }; // mov al, 1; retn
	WriteBytesASM(0x47FB10, patch_can_display_tag, 0x3);

	BYTE patch_file_extenstion_check[0x4] = { 0xE9, 0xCD, 0x00, 0x00 }; // mov al, 1; retn
	WriteBytesASM(0x47FB8D, patch_file_extenstion_check, 0x4);

	// End patches copied from opensauce

	// Doesn't fix the underlying issue, just removes the error message allowing one more scenerio to be opened
	BYTE patch_out_of_resources[0x2] = { 0xEB, 0x03 };
	WriteBytesASM(0x44CF1F, patch_out_of_resources, 0x2);
	NopFill(0x44CDD0, 0x5);

#pragma endregion

#pragma region Hooks
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	toggle_expert_mode_orginal = CAST_PTR(toggle_expert_mode,0x4024A0);
	DetourAttach(&(PVOID&)toggle_expert_mode_orginal, toggle_expert_mode_hook);

	LoadMenuOrginal = LoadMenuW;
	DetourAttach(&(PVOID&)LoadMenuOrginal, LoadMenuHook);

	CCmdUI__Enable_Orginal = CAST_PTR(CCmdUI__Enable, 0x67F09A);
	DetourAttach(&(PVOID&)CCmdUI__Enable_Orginal, CCmdUI__Enable_Hook);

	CCmdTarget__OnCmdMsg_Orginal = CAST_PTR(CCmdTarget__OnCmdMsg, 0x67EE0F);
	DetourAttach(&(PVOID&)CCmdTarget__OnCmdMsg_Orginal, CCmdTarget__OnCmdMsg_hook);
	DetourTransactionCommit();
#pragma endregion

	memset(CAST_PTR(void*,0x9AF809), 1, 1); // set is_expert_mode to one
	update_field_display();
	toggle_display_templates();
	main_menu = LoadMenuA(g_hModule, MAKEINTRESOURCEA(GUERILLA_MENU));
}

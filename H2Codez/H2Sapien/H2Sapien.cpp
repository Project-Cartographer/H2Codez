#include "../stdafx.h"
#include "H2Sapien.h"
#include "..\Common\H2EKCommon.h"
#include "..\HaloScript\hs_interface.h"
#include "..\Common\BlamBaseTypes.h"
#include "..\util\Patches.h"
#include "..\Resources\resource.h"
#include <Shellapi.h>
#include <iostream>
#include <fstream>
#include <D3D9.h>
#include "Console.h"
#include "TagUpdate.h"

using namespace HaloScriptCommon;

typedef int (__thiscall *WTL_CWindow_Input)(void *thisptr, int a2, UINT uMsg, int hMenu, LPARAM lParam, int *a6, int a7);
WTL_CWindow_Input main_window_input_orginal;

typedef HMENU(WINAPI *LoadMenuTypedef)(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName);
static LoadMenuTypedef LoadMenuOrginal;

HMENU new_menu;

static HMENU WINAPI LoadMenuHook(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName)
{
	if (hInstance == GetModuleHandle(NULL)) {
		return new_menu;
	}
	return LoadMenuOrginal(hInstance, lpMenuName);
}

bool running_game_scripts = false;
bool using_in_game_settings = false;

inline static void CheckItem(UINT item, bool enable)
{
	CheckMenuItem(new_menu, item, MF_BYCOMMAND | (enable ? MF_CHECKED : MF_UNCHECKED));
}

video_settings halo2_video_settings;
video_settings sapien_defaults;

void apply_video_settings()
{
	CheckItem(SAPIEN_IN_GAME_LOD, using_in_game_settings);

	WriteValue(0x00A5D134, using_in_game_settings ? halo2_video_settings : sapien_defaults);
}

char command_script[0x100];
INT_PTR CALLBACK SapienRunCommandProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE) {
		EndDialog(hwndDlg, 0);
		return true;
	}
	if (uMsg == WM_COMMAND && LOWORD(wParam)) {
		if (GetDlgItemText(hwndDlg, SAPIEN_COMMAND, command_script, sizeof(command_script)))
			H2SapienConsole::run_hs_command(command_script);
		EndDialog(hwndDlg, 0);
		return false;
	}
	return false;
}

// TODO: fix MessageBoxA refuseing to work right.
INT_PTR CALLBACK CustomDirectorSpeed(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (uMsg == WM_COMMAND) {
		if (wParam == IDOK) {
			char speed_text[200];
			if (GetDlgItemText(hwndDlg, IDC_CUSTOM_SPEED, speed_text, sizeof(speed_text)))
			{
				try {
					float new_speed = std::stof(speed_text);
					new_speed = std::fmin(std::fabs(new_speed), 5000.0f);
					WriteValue(0x009AAC60, new_speed);
					H2SapienConsole::print("speed is now " + std::to_string(new_speed));
				}
				catch (invalid_argument ex) {
					//MessageBoxA(hwndDlg, "Not a valid number!", "ERROR!", MB_OK | MB_SETFOREGROUND);
				}
				catch (out_of_range ex) {
					//MessageBoxA(hwndDlg, "Number out of range!", "ERROR!", MB_OK | MB_SETFOREGROUND);
				}
			}
			EndDialog(hwndDlg, 0);
			return 0;
		}
	}
	return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

int __fastcall main_window_input_hook(void *thisptr, BYTE _, int a2, UINT uMsg, int wParam, LPARAM lParam, int *subfunction_out, int handled)
{
	if (uMsg == WM_COMMAND && !handled) {
		switch (wParam) {
			case SAPIEN_FILE_NEWINSTANCE:
			{
				return H2CommonPatches::newInstance();
			}
			case SAPIEN_OPEN_RUN_COMMAND_DIALOG:
			{
				DialogBoxParam(g_hModule, MAKEINTRESOURCE(SAPIEN_COMMAND_DIALOG), 0, SapienRunCommandProc, 0);
				return 1;
			}
			case ID_VIEW_CUSTOMDIRECTORSPEED:
			{
				DialogBoxParam(g_hModule, MAKEINTRESOURCE(IDD_SELECT_CUSTOM_SPEED), 0, CustomDirectorSpeed, 0);
				return 1;
			}
			case SAPIEN_SCRIPT_DOC:
			{
				H2CommonPatches::generate_script_doc();
				return 1;
			}
			case SAPIEN_IN_GAME_LOD:
			{
				typedef void(__cdecl *update_video_settings)(char a1);
				update_video_settings update_video_settings_impl = reinterpret_cast<update_video_settings>(0x006FBCF0);

				using_in_game_settings = !using_in_game_settings;
				apply_video_settings();

				conf.setBoolean("in_game_lod", using_in_game_settings);

				update_video_settings_impl(0);

				return 1;
			}
			case 32837:
			{
				char expert_mode = *CAST_PTR(char*, 0xA68319);
				conf.setBoolean("expert_mode", expert_mode);
				break;
			}
			case 32870:
			{
				running_game_scripts = !running_game_scripts;
				conf.setBoolean("running_game_scripts", running_game_scripts);
				CheckItem(32870, running_game_scripts);
				break;
			}
		}
	}
	return main_window_input_orginal(thisptr, a2, uMsg, wParam, lParam, subfunction_out, handled);
}

std::string baggage_name;

errno_t __cdecl fopen_s_baggage_hook(FILE **File, const char *Filename, const char *Mode)
{
	baggage_name = H2CommonPatches::get_temp_name("baggage.txt");
	return fopen_s(File, baggage_name.c_str(), "w");
}

int __cdecl fclose_baggage_hook(FILE *File)
{
	int ret_data = fclose(File);
	ShellExecuteA(NULL, NULL, baggage_name.c_str(), NULL, NULL, SW_SHOW);
	return ret_data;
}

errno_t print_help_to_doc()
{
	H2CommonPatches::generate_script_doc("hs_doc.txt");
	return 0;
}

void **__cdecl status_func_impl(int command_id, void *a2, char a3)
{
	ofstream output;
	std::string temp_file_name = H2CommonPatches::get_temp_name("status.txt");

	output.open(temp_file_name, ios::out);
	if (output)
	{
		for (hs_global_variable *current_var : g_halo_script_interface->global_table)
		{
			std::string value_as_string = get_value_as_string(current_var->variable_ptr, current_var->type);
			output << current_var->name << "   :    " << value_as_string << std::endl;
		}
	}
	output.close();
	ShellExecuteA(NULL, NULL, temp_file_name.c_str(), NULL, NULL, SW_SHOW);
	return HaloScriptCommon::epilog(a2, 0);
}

signed int get_tick_rate()
{
	WORD *tick_rate = reinterpret_cast<WORD*>(0x00A6C700);
	return *tick_rate;
}

bool __cdecl scripts_disabled()
{
	return !running_game_scripts;
}

void __cdecl process_decoration_brush_input__hook(int *a1, int *a2, char left_pressed, char right_pressed)
{
	typedef void(__cdecl *process_decoration_brush_input)(int *a1, int *a2, char left_pressed, char right_pressed);
	process_decoration_brush_input process_decoration_brush_input_impl = reinterpret_cast<process_decoration_brush_input>(0x4A8ED0);

	process_decoration_brush_input_impl(a1, a2, left_pressed, right_pressed);

	if (left_pressed || right_pressed)
		WriteValue(0xFE8CE4, 1); // not sure how but this reset something and makes the new decor render correctly.
}

char __cdecl set_scenario_path_hook(LPCWSTR path_passed_to_us)
{
	typedef char (__cdecl *set_scenario_path)(LPCWSTR path);
	auto set_scenario_path_impl = reinterpret_cast<set_scenario_path>(0x00458E40);
	if (!set_scenario_path_impl(path_passed_to_us)) // support standard path escaping
	{
		int argwc = *reinterpret_cast<int*>(0x010E6A7C);
		wchar_t **argwv = *reinterpret_cast<wchar_t ***>(0x010E6A84);
		if (argwc >= 2)
			return set_scenario_path_impl(argwv[1]);
		return false;
	}
	return true;
}

__declspec(naked) void hierarchy_selection_code__cmp_hook()
{
	__asm {
		cmp eax, eax // set zero flag
		ret
	}
}

int __stdcall game_view_true_stub(int)
{
	return true;
}


hs_command status_cmd(
	"status",
	hs_type::nothing,
	hs_default_func_check,
	status_func_impl,
	"dumps the value of all global status variables to file."
);

template <typename value_type>
inline void GetHalo2DisplaySetting(const char *name, value_type &result)
{
	HKEY video_settings;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Halo 2\\Video Settings", 0, KEY_READ, &video_settings) == ERROR_SUCCESS) {
		DWORD size = sizeof(DWORD);
		DWORD value;
		if (RegGetValueA(video_settings, NULL, name, RRF_RT_REG_DWORD, NULL, &value, &size) == ERROR_SUCCESS)
			result = static_cast<value_type>(value);
		RegCloseKey(video_settings);
	}
}

void InitHalo2DisplaySettings()
{
	GetHalo2DisplaySetting("AntiAliasing", halo2_video_settings.AntiAliasing);
	GetHalo2DisplaySetting("AspectRatio", halo2_video_settings.AspectRatio);
	GetHalo2DisplaySetting("Brightness", halo2_video_settings.Brightness);
	GetHalo2DisplaySetting("DisplayMode", halo2_video_settings.DisplayMode);
	GetHalo2DisplaySetting("Gamma", halo2_video_settings.Gamma);
	GetHalo2DisplaySetting("HubArea", halo2_video_settings.HubArea);
	GetHalo2DisplaySetting("LevelOfDetail", halo2_video_settings.LevelOfDetail);
	GetHalo2DisplaySetting("SafeArea", halo2_video_settings.SafeArea);
	GetHalo2DisplaySetting("ScreenRefresh", halo2_video_settings.ScreenInfo.refresh_rate);
	GetHalo2DisplaySetting("ScreenResX", halo2_video_settings.ScreenInfo.x);
	GetHalo2DisplaySetting("ScreenResY", halo2_video_settings.ScreenInfo.y);
}

void H2SapienPatches::Init()
{
	// apply in-game console patches
	ConsoleInit();
	// setup tag sync
	StartTagSync();
	// set current directory to executable path
	std::wstring new_current_dir = H2CommonPatches::GetExeDirectoryWide();
	SetCurrentDirectoryW(new_current_dir.c_str());
#pragma region value init
	hs_command **command_table = reinterpret_cast<hs_command **>(0x9E9E90);
	hs_global_variable **global_table = reinterpret_cast<hs_global_variable **>(0x9ECE28);
	g_halo_script_interface->init_custom(command_table, global_table);
	g_halo_script_interface->RegisterCommand(hs_opcode::status, &status_cmd);

	new_menu = LoadMenu(g_hModule, MAKEINTRESOURCE(SAPIEN_MENU));
	InitHalo2DisplaySettings();

	using_in_game_settings = conf.getBoolean("in_game_lod", 0);
	WriteValue<BYTE>(0xA68319, conf.getBoolean("expert_mode", 1)); // set is_expert_mode to one

	running_game_scripts = conf.getBoolean("running_game_scripts", 0);
	CheckItem(32870, running_game_scripts);

	WriteValue<double>(0x00F84D30, 5.0); // CPUScore; used to decide when to force low lod
	WriteValue(0x00F84D28, conf.getBoolean("AllowVsync", 1));
	WriteValue(0x00F84D24, conf.getBoolean("CinematicShadow", 1));

	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof(statex);

	GlobalMemoryStatusEx(&statex);

	int SystemMemory = static_cast<int>(statex.ullAvailPhys / (1024 * 1024));
	int VideoMemory = conf.getNumber("VideoMemory", 100) * 1024 * 1024; // megabytes --> bytes
	int use_hardware_vertexprocessing = conf.getBoolean("use_hardware_vertexprocessing", true);
	WriteValue(0xF84D1C, SystemMemory);
	WriteValue(0xF84D18, VideoMemory);
	WriteValue(0xF84D10, use_hardware_vertexprocessing);
	apply_video_settings();

#pragma endregion

#pragma region Patches
	// Stop sapien from getting a mutex on the main directory
	NopFill(0x409D3D, 0xD);

	// replace bultin IO since it seems to crash
	PatchCall(0x00477AE8, fopen_s_baggage_hook);
	PatchCall(0x00477D4F, fclose_baggage_hook);

	PatchCall(0x00477C05, fwprintf); // scenario name
	PatchCall(0x00477C54, fwprintf);
	PatchCall(0x00477D25, fwprintf); // ==block sizes==
	PatchCall(0x00477D45, fwprintf);

	// %ws should really be used for wchar_t strings instead of %s
	WritePointer(0x477C4F, L"%ws\n\n");
	WritePointer(0x477D40, L"%ws\n");

	PatchCall(0x5783B0, print_help_to_doc);

	// don't clear the console contents when closed
	NopFill(0x4ECD7C, 5);

	// allow other processes to read files open with fopen_s
	WriteValue(0x00738FF3 + 1, _SH_DENYWR);

	// fixes debug_tags command messing up string formating
	// and getting a werid path directly in program files
	const char *tag_debug_format = "%S//%s_tag_dump.txt";
	WriteValue(0x004B5F33 + 1, tag_debug_format);

	// Replace pointers to the commmand table
	static DWORD cmd_offsets[] = 
	{
		0x004E2225 + 3, 0x004E23F0 + 3,  0x004E2414 + 3,
		0x004E2701 + 3, 0x004E2DF4 + 3, 0x004E3014 + 3,
		0x004E304D,     0x004E30FB
	};

	hs_command **cmds = g_halo_script_interface->get_command_table();

	for (DWORD addr : cmd_offsets)
		WritePointer(addr, cmds);

	// patch command table size
	const static int hs_cmd_table_size = g_halo_script_interface->get_command_table_size();
	WriteValue(0x008EB118, hs_cmd_table_size);

	// Replace pointers to the globals table

	static DWORD var_offsets[] =
	{
		0x004E2295, 0x004E22B0, 0x004E22F0,
		0x004E2334, 0x004E27B1, 0x00635A11,
		0x00635A2D, 0x00635A7D, 0x00635AFB,
		0x00635B12
	};

	hs_global_variable **vars = g_halo_script_interface->get_global_table();

	for (DWORD addr : var_offsets)
		WriteValue(addr + 3, vars);

	// patch globals table size
	const static int hs_global_table_size = g_halo_script_interface->get_global_table_size();
	WriteValue(0x008EFDB4, hs_global_table_size);

	// fix "game_tick_rate"
	WriteJmp(0x006F7D60, get_tick_rate);

	WriteValue(0x00A5D104, sapien_defaults);

	PatchCall(0x0052C516, scripts_disabled);

	if (conf.getBoolean("decoration_force_update", false))
		PatchCall(0x004876DE, process_decoration_brush_input__hook);

	// fix sapien not working well with shell
	PatchCall(0x0046361A, set_scenario_path_hook);

	// ...
	if (conf.getBoolean("use_loading_animation", false))
	{
		NopFill(0x004A7730, 2);
		NopFill(0x004A7A36, 2);
	}

#ifdef _DEBUG
	WriteCall(0x004882B1, hierarchy_selection_code__cmp_hook); // player_simulation
	WriteCall(0x00485280, hierarchy_selection_code__cmp_hook); // ai_path
	WriteCall(0x00488686, hierarchy_selection_code__cmp_hook); // leaf_debug
	WriteCall(0x0048781D, hierarchy_selection_code__cmp_hook); // decorator_paint
	WriteCall(0x0048D45D, hierarchy_selection_code__cmp_hook); // world_measure

	WritePointer(0x00802894, game_view_true_stub);
	WritePointer(0x008028A4, game_view_true_stub);
	WritePointer(0x008028C4, game_view_true_stub);
	WritePointer(0x0080296C, game_view_true_stub);
#endif // DEBUG

#pragma endregion

#pragma region Hooks

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	main_window_input_orginal = CAST_PTR(WTL_CWindow_Input, 0x475B60);
	DetourAttach(&(PVOID&)main_window_input_orginal, main_window_input_hook);

	LoadMenuOrginal = LoadMenuW;
	DetourAttach(&(PVOID&)LoadMenuOrginal, LoadMenuHook);

	DetourTransactionCommit();
#pragma endregion
}
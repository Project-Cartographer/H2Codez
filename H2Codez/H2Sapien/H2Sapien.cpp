#include "stdafx.h"
#include "H2Sapien.h"
#include "Common\H2EKCommon.h"
#include "Common\BlamBaseTypes.h"
#include "util\Patches.h"
#include "Resources\sapien_accelerators.h"
#include "Resources\resource.h"
#include "Console.h"
#include "TagUpdate.h"
#include "Profile.h"
#include "RenderDebug.h"
#include "HaloScript.h"
#include <unordered_set>
#include <Shellapi.h>
#include <iostream>
#include <fstream>

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
typedef HACCEL (__stdcall *LoadAcceleratorsW_T)(HINSTANCE hInstance, LPCWSTR lpTableName);
LoadAcceleratorsW_T LoadAcceleratorsWOrg;
HACCEL __stdcall LoadAcceleratorsWHook(HINSTANCE hInstance, LPCWSTR lpTableName)
{
	if (hInstance == GetModuleHandle(NULL)) {
		return LoadAcceleratorsWOrg(g_hModule, lpTableName);
	}
	return LoadAcceleratorsWOrg(hInstance, lpTableName);
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

video_settings *read_game_video_settings()
{
	return reinterpret_cast<video_settings*>(0x00A5D134);
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
					H2SapienConsole::print("speed is now x" + std::to_string(new_speed));
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
			case SAPIEN_TILE_VERTICAL:
			{
				SendMessageW(((HWND*)thisptr)[2], WM_MDITILE, MDITILE_VERTICAL, 0);
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

bool __cdecl is_render_enabled()
{
	char no_renderer = *reinterpret_cast<char*>(0xA68318);
	return no_renderer == 0;
}

wdp_type __cdecl wdp_initialize()
{
	if (conf.getBoolean("simulate_game", false)) // see if we should pretend to be game
		return wdp_type::_game;
	return is_render_enabled() ? wdp_type::_sapien : wdp_type::_tool;
}

bool __cdecl is_sapien()
{
	return wdp_initialize() == wdp_type::_sapien;
}

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

void H2SapienPatches::InitDisplaySettings()
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

	// fix game profiles
	fix_game_save();

	render_debug_info_init();

	haloscript_init();
	// set current directory to executable path
	std::wstring new_current_dir = H2CommonPatches::GetExeDirectoryWide();
	SetCurrentDirectoryW(new_current_dir.c_str());
#pragma region value init

	new_menu = LoadMenu(g_hModule, MAKEINTRESOURCE(SAPIEN_MENU));
	InitDisplaySettings();

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
	if (SystemMemory < 1200)
	{
		MessageBoxA(NULL, "At least 1200 megabytes of free memory is recommanded when running sapien.", "Warning : Low memory", MB_OK);
	}
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

	// don't clear the console contents when closed
	NopFill(0x4ECD7C, 5);

	// allow other processes to read files open with fopen_s
	WriteValue(0x00738FF3 + 1, _SH_DENYWR);

	// fixes debug_tags command messing up string formating
	// and getting a werid path directly in program files
	const char *tag_debug_format = "%S//%s_tag_dump.txt";
	WriteValue(0x004B5F33 + 1, tag_debug_format);

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

	// game_view experiments
#if _DEBUG
	//WriteCall(0x00485280, hierarchy_selection_code__cmp_hook); // ai_path
	WriteCall(0x00488686, hierarchy_selection_code__cmp_hook); // leaf_debug

	WritePointer(0x00802894, game_view_true_stub);
	//WritePointer(0x008028A4, game_view_true_stub);
	//WritePointer(0x008028C4, game_view_true_stub);
	//WritePointer(0x0080296C, game_view_true_stub);
#endif // DEBUG

	// replace structure painter with something more fun
	WriteValue(0x00416028 + 4, 'play');
	WriteValue(0x00416072 + 1, 'play');

	// allow dumping screen print to standard output
	WriteValue(0xAAC0BD, conf.getBoolean("dump_screen_print_to_console", is_debug_build()));
	NopFillRange(0x50448A, 0x504491);

	WriteJmp(0x458940, is_sapien); // default is_sapien

	// used is_sapien/is_render_enabled
	WriteJmp(0x409A40, wdp_initialize);

	NopFill(0x6FC641, 5); // update_display crashes if called too earlier
	NopFill(0x64E3B8, 2); // always enable editor_strip_dialogue_sounds
	PatchCall(0x006FCA65, is_render_enabled); // technically a call to wdp_initialize, but just need this to return something other than 2

	std::unordered_set<DWORD> patch_to_simulate_game  {
		//0x005D28F0, // verify_physics_maybe
		0x005A2321, // player_effect_setup_scenario
		0x006EE31E, // rasterizer_initialize
		0x006B9DF0, // c_decorator_placement_definition_setup_sbsp
		0x004F1D15, // director_setup_scenario
		0x00682E63, // ???
		0x00643A5A, // ???
		0x004B5AE7, // tag_reload
		0x004CAD26, // update_input
		0x004CAF24, // update_input
		0x00514C9D, // havok_setup_sbsp
	};

	for (auto addr : patch_to_simulate_game)
	{
		PatchCall(addr, is_render_enabled);
	}


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
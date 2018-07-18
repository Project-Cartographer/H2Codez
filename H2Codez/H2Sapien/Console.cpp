#include "Console.h"
#include "..\util\RingBuffer.h"
#include "..\stdafx.h"
#include "..\Common\H2EKCommon.h"

inline bool is_ctrl_down()
{
	return HIBYTE(GetKeyState(VK_CONTROL));
}

RingBuffer<std::string> console_history(8);
int history_view_location = 0;
bool history_view_inital = true;
bool history_disable = false;

void H2SapienConsole::run_hs_command(const std::string &script)
{
	if (!history_disable)
		console_history.push(script);
	HaloScriptCommon::hs_execute(script.c_str());
}

void update_console_state()
{
	typedef void(__cdecl *_t_update_console_state)(int console_state);
	auto update_console_state_impl = reinterpret_cast<_t_update_console_state>(0x58F580);
	update_console_state_impl(0xA9F630); // hardcoded console state offset
}

inline void console_close()
{
	typedef void(*t_console_close)();
	auto console_close_impl = reinterpret_cast<t_console_close>(0x004EBE50);
}

void copy_from_console_history()
{
	history_view_inital = false;
	char *console_input = reinterpret_cast<char*>(0xA9F52C);
	if (!console_history.empty())
	{
		strncpy(console_input, console_history.get(history_view_location).c_str(), 0x100);
		update_console_state();
	}
}

bool __stdcall on_console_input(WORD keycode)
{
	printf("key  :  %d\n", keycode);
	printf("key (low)  :  %d\n", LOBYTE(keycode));
	printf("key (high)  :  %d\n", HIBYTE(keycode));

	char *console_input = reinterpret_cast<char*>(0xA9F52C);
	WORD *cursor_pos = reinterpret_cast<WORD*>(0xa9f636);
	HWND *main_hwnd = reinterpret_cast<HWND *>(0x00A68B9C);

	printf("console: %s \n", console_input);

	switch (keycode) {
	case 263: // `
		history_view_location = 0;
		history_view_inital = true;
		console_close();
		return false;
	case VK_RETURN:
	case 262: // somehow sapien encodes enter as this
		history_view_location = 0;
		history_view_inital = true;
		if (strnlen_s(console_input, 0x100) > 0)
		{

			update_console_state();
		}
		else {
			console_close();
		}
		return true;
	case VK_DELETE:
		SecureZeroMemory(console_input, 0x100);
		update_console_state();
		H2SapienConsole::print("cleared console.");
		break;
	case VK_UP:
		if (history_disable) return true;

		if (!history_view_inital)
			history_view_location--;
		copy_from_console_history();
		return true;
	case VK_DOWN:
		if (history_disable) return true;

		history_view_location++;
		copy_from_console_history();
		return true;
	case 'C':
		if (is_ctrl_down()) {
			if (H2CommonPatches::copy_to_clipboard(console_input))
				H2SapienConsole::print("copied to clipboard!");
			update_console_state();
		}
		break;
	case 'V':
		std::string new_text;
		if (is_ctrl_down() && H2CommonPatches::read_clipboard(new_text)) {
			strncpy(console_input, new_text.c_str(), 0x100);
			H2SapienConsole::print("pasted from clipboard!");
			update_console_state();
		}
		break;
	}
	return false;
}

// hooks a switch statement that handles speical key presses (e.g. enter, tab)
__declspec(naked) void console_input_jump_hook()
{
	__asm {
		// save register
		push eax

		// undo add
		sub eax, 0xFFFFFFF7
		// pass eax (keycode) to our code
		push eax
		call on_console_input
		// check if default code should be skipped
		cmp eax, 1
		jz SKIP


DEFAULT :

		// restore registers
		pop eax

		// replaced code
		cmp eax, 254

		// jump back to sapien code
		push 0x004ECC33
		ret

SKIP :
		pop eax
		push 0x004ECD24
		ret
	}
}

int console_write_return_addr;
int memcpy_impl = 0x4ADDC0;
// hooks the function that handles writing keypresses to console buffer if printable
__declspec(naked) void console_write_hook()
{
	__asm {
		// backup the return address
		pop eax
		mov console_write_return_addr, eax

		// replaced code
		call memcpy_impl

		// get keycode and check
		mov al, [ebx + 1]
		cmp al, 0x60 // ascii '`'

		// ignore input
		jz ignore_input
		// return to normal execution
		jmp end_function

ignore_input :
		add esp, 0x18
		// push new return addr pointing to function epilog
		mov console_write_return_addr, 0x58F85E

end_function :
		jmp console_write_return_addr
	}
}

void H2SapienPatches::ConsoleInit()
{
	WriteJmp(0x4ECC2E, &console_input_jump_hook);
	// replace a call to memcpy
	PatchCall(0x58F6AA, &console_write_hook);

	int history_size = conf.getNumber("console_history_size", 8);
	if (history_size > 0)
		console_history.resize(history_size);
	else
		history_disable = true;
}
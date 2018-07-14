#include "FiloInterface.h"

namespace FiloInterface
{
	void init_filo(filo *data, std::string path, bool mode)
	{
		typedef filo *(__cdecl *filo__init_filo)(filo *data_struct, const char *path, char mode);
		DWORD func_offset = SwitchAddessByMode(0x52A540, 0x4B8080, 0x48BEA0);
		auto filo__init_filo_impl = reinterpret_cast<filo__init_filo>(func_offset);

		filo__init_filo_impl(data, path.c_str(), mode);
	}

	bool is_read_only(filo *data)
	{
		typedef bool(__cdecl *filo__is_read_only)(filo *a1);
		DWORD func_offset = SwitchAddessByMode(0x528010, 0x4BC400, 0x48C5D0);
		auto filo__is_read_only_impl = reinterpret_cast<filo__is_read_only>(func_offset);

		return filo__is_read_only_impl(data);
	}

	bool create(filo *data)
	{
		typedef bool(__cdecl *filo__create)(filo *a1);
		DWORD func_offset = SwitchAddessByMode(0x528B10, 0x4BCF00, 0x48D140);
		auto filo__create_impl = reinterpret_cast<filo__create>(func_offset);

		return filo__create_impl(data);
	}

	bool delete_existing(filo *data)
	{
		typedef bool(__cdecl *filo__delete)(filo *data_struct);
		DWORD func_offset = SwitchAddessByMode(0x528E70, 0x4BD260, 0x48BEA0);
		auto filo__delete_impl = reinterpret_cast<filo__delete>(func_offset);

		return filo__delete_impl(data);
	}

	bool check_access(filo *data)
	{
		typedef bool(__cdecl *filo__check_access)(filo *a1);
		DWORD func_offset = SwitchAddessByMode(0x529020, 0x4BD410, 0x48D650);
		auto filo__check_access_impl = reinterpret_cast<filo__check_access>(func_offset);

		return filo__check_access_impl(data);
	}

	bool open(filo *data, __int16 mode, DWORD *error_code)
	{
		typedef bool(__cdecl *filo__open)(filo *filo, __int16 mode, DWORD *error_code);
		DWORD func_offset = SwitchAddessByMode(0x5291B0, 0x4BD5A0, 0x48D7E0);
		auto filo__open_impl = reinterpret_cast<filo__open>(func_offset);

		return filo__open_impl(data, mode, error_code);
	}

	bool close(filo *data)
	{
		typedef bool(__cdecl *filo__close)(filo *a1);
		DWORD func_offset = SwitchAddessByMode(0x5295D0, 0x4BD9C0, 0x48DC00);
		auto filo__close_impl = reinterpret_cast<filo__close>(func_offset);

		return filo__close_impl(data);
	}

	bool set_position(filo *data, LONG lDistanceToMove, bool hide_error_from_user)
	{
		typedef bool (__cdecl *filo__set_position)(filo *data, LONG lDistanceToMove, char hide_error_from_user);
		DWORD func_offset = SwitchAddessByMode(0x5296B0, 0x4BDAA0, 0x48DCE0);
		auto filo__set_position_impl = reinterpret_cast<filo__set_position>(func_offset);

		return filo__set_position_impl(data, lDistanceToMove, hide_error_from_user);
	}

	DWORD get_eof(filo *data)
	{
		typedef char(__cdecl *filo__close)(filo *a1);
		DWORD func_offset = SwitchAddessByMode(0x5297B0, 0x4BDBA0, 0x48DDE0);
		auto filo__close_impl = reinterpret_cast<filo__close>(func_offset);

		return filo__close_impl(data);
	}

	bool set_eof(filo *data, LONG lDistanceToMove)
	{
		typedef bool(__cdecl *filo__set_eof)(filo *data, LONG lDistanceToMove);
		DWORD func_offset = SwitchAddessByMode(0x529890, 0x4BDC80, 0x48DEC0);
		auto filo__set_eof_impl = reinterpret_cast<filo__set_eof>(func_offset);

		return filo__set_eof_impl(data, lDistanceToMove);
	}

	bool read(filo *filo_ptr, LPVOID data_buffer, DWORD nNumberOfBytesToRead, bool hide_errors_from_user)
	{
		typedef char (__cdecl *filo__read)(filo *filo_ptr, DWORD nNumberOfBytesToRead, char hide_errors_from_user, LPVOID lpBuffer);
		DWORD func_offset = SwitchAddessByMode(0x529980, 0x4BDD70, 0x48DFB0);
		auto filo__read_impl = reinterpret_cast<filo__read>(func_offset);

		return filo__read_impl(filo_ptr, nNumberOfBytesToRead, hide_errors_from_user, data_buffer);
	}

	bool write(filo *filo_ptr, LPVOID data, size_t data_size)
	{
		typedef bool(__cdecl *filo__write)(filo *a1, DWORD nNumberOfBytesToWrite, LPVOID lpBuffer);
		DWORD func_offset = SwitchAddessByMode(0x529B10, 0x4BDF00, 0x48DFB0);
		auto filo__write_impl = reinterpret_cast<filo__write>(func_offset);

		return filo__write_impl(filo_ptr, data_size, data);
	}

	bool get_last_write(filo *filo_ptr, FILETIME *last_write_time)
	{
		typedef bool(__cdecl *filo__get_last_write)(filo *filo_ptr, _FILETIME *last_write_time);
		DWORD func_offset = SwitchAddessByMode(0x529CA0, 0x4BE090, 0x48E2D0);
		auto filo__get_last_write_impl = reinterpret_cast<filo__get_last_write>(func_offset);

		return filo__get_last_write_impl(filo_ptr, last_write_time);
	}

	bool get_size(filo *data, DWORD *file_size)
	{
		typedef bool(__cdecl *filo__get_size)(filo *filo_ptr, DWORD *file_size);
		DWORD func_offset = SwitchAddessByMode(0x529E50, 0x4BE240, 0x48E480);
		auto filo__get_size_impl = reinterpret_cast<filo__get_size>(func_offset);

		return filo__get_size_impl(data, file_size);
	}

	std::string get_path_info(filo *data, PATH_FLAGS flags)
	{
		typedef BYTE *(_cdecl *get_path_info_from_filo)(filo*, char, char*);
		DWORD func_offset = SwitchAddessByMode(0x52A310, 0x4B7E80, 0x48BCC0);
		auto get_path_info_from_filo_impl = CAST_PTR(get_path_info_from_filo, func_offset);

		char out_buffer[0x100];
		get_path_info_from_filo_impl(data, flags, out_buffer);
		return out_buffer;
	}
}
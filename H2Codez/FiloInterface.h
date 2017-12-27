#pragma once
#include "stdafx.h"

struct filo
{
	unsigned long			signature;
	unsigned short      	flags;
	signed short     		location;
	char                    path[256];
	HANDLE		            handle;
	HRESULT		            api_result;
};
BOOST_STATIC_ASSERT(sizeof(filo) == 0x110);

enum PATH_FLAGS : BYTE {
	CONTAINING_DIRECTORY_PATH = 1, // base
	CONTAINING_DIRECTORY_NAME = 2, // base
	FILE_NAME = 4, // base
	FILE_EXTENSION = 8, // base

	FULL_PATH = (FILE_NAME | CONTAINING_DIRECTORY_PATH | FILE_EXTENSION),
};

inline PATH_FLAGS operator|(PATH_FLAGS a, PATH_FLAGS b)
{
	return static_cast<PATH_FLAGS>(static_cast<int>(a) | static_cast<int>(b));
}

namespace  FiloInterface
{
	void init_filo(filo *data, std::string path, bool mode);

	/* Returns true if it is read-only */
	bool is_read_only(filo *data);

	/* 
	Creates an empty file or path (depends on filo flags), all intermediate directories are created
	Returns success
	*/
	bool create(filo *data);

	/* Deletes the file or directory pointed to by the filo, returns success */
	bool delete_existing(filo *data);

	/* Returns true if the path exists and we can access it */
	bool check_access(filo *data);

	/* Returns success */
	bool open(filo *data, __int16 mode, DWORD *error_code);

	/* Returns success */
	bool close(filo *data);

	/* Returns success */
	bool set_position(filo *data, LONG lDistanceToMove, bool hide_error_from_user);

	/* Returns file size or INVALID_FILE_SIZE on failure. Filo needs to be opened before use. */
	DWORD get_eof(filo *data);

	/* Returns success */
	bool set_eof(filo *data, LONG lDistanceToMove);

	/*
	On success the data read is written to data_buffer and the function returns true
	On failure, if hide_errors_from_user is set to false an error is displayed to the user and false is returned, if the number of bytes read doesn't match the requested amount ERROR_HANDLE_EOF is set
	*/
	bool read(filo *filo_ptr, LPVOID data_buffer, DWORD nNumberOfBytesToRead, bool hide_errors_from_user);

	/* Returns success */
	bool write(filo *filo_ptr, LPVOID data, size_t data_size);

	/* Returns sucess, last_write_time is set to the file modification data if sucessful otherwise it's not modified */
	bool get_last_write(filo *filo, FILETIME *last_write_time);

	/* Returns success */
	bool get_size(filo *data, DWORD *file_size);

	std::string get_path_info(filo *data, PATH_FLAGS flags);
};

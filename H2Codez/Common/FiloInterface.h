#pragma once
#pragma pack(1)

#include "stdafx.h"
#include "BlamBaseTypes.h"
#include "util/string_util.h"

struct file_reference
{
	enum _flags : signed short
	{
		no_flags = 0,
		filename = 1
	};

	blam_tag				signature = 'filo';
	signed short	      	flags = no_flags;
	signed short     		location = -1;
	char                    path[256] = {};
	HANDLE		            handle = nullptr;
	HRESULT		            api_result = ERROR_SUCCESS;

	constexpr file_reference()
	{
	}

	constexpr file_reference(signed short _location):
		location(_location)
	{
	}

	file_reference(const char *path, bool is_directory)
	{
		append(path, is_directory);
	}

	file_reference(const std::string &path, bool is_directory)
	{
		append(path.c_str(), is_directory);
	}


	inline void add_directory(const char *directory)
	{
		ASSERT_CHECK(directory != nullptr);
		append_name_to_path(path, directory);
	}

	inline void add_directory(const std::string &directory)
	{
		add_directory(directory.c_str());
	}

	inline void set_file_name(const char *name)
	{
		ASSERT_CHECK(name != nullptr);
		if (flags & filename)
			remove_last_part_of_path(path);
		append_name_to_path(path, name);
		flags |= filename;
	}

	inline void append(const char *path, bool is_directory)
	{
		ASSERT_CHECK(path != nullptr);
		if (is_directory)
			add_directory(path);
		else
			set_file_name(path);
	}
};
CHECK_STRUCT_SIZE(file_reference, 0x110);

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

enum FILO_OPEN_OPTIONS : __int16
{
	FILO_OPEN_READ = 0x1,
	FILO_RANDOM_ACCESS = 0x1,
	filo_open_mode_2 = 0x2,
	filo_open_mode_4 = 0x4,
	filo_open_mode_8 = 0x8,
	FILO_HIDE_ERRORS = 0x10,
	FILO_TEMPORARY = 0x20,
	FILO_DELETE_ON_CLOSE = 0x40,
	FILO_SEQUENTIAL_SCAN = 0x100,
};


namespace  FiloInterface
{
	/* Returns true if it is read-only */
	bool is_read_only(file_reference *data);

	/* 
	Creates an empty file or path (depends on file_reference flags), all intermediate directories are created
	Returns success
	*/
	bool create(file_reference *data);

	/* Deletes the file or directory pointed to by the file_reference, returns success */
	bool delete_existing(file_reference *data);

	/* Returns true if the path exists and we can access it */
	bool check_access(file_reference *data);

	/* Returns success */
	bool open(file_reference *data, FILO_OPEN_OPTIONS mode, DWORD *error_code = nullptr);

	/* Returns success */
	bool close(file_reference *data);

	/* Returns success */
	bool set_position(file_reference *data, LONG lDistanceToMove, bool hide_error_from_user);

	/* Returns file size or INVALID_FILE_SIZE on failure. Filo needs to be opened before use. */
	DWORD get_eof(file_reference *data);

	/* Returns success */
	bool set_eof(file_reference *data, LONG lDistanceToMove);

	/*
	On success the data read is written to data_buffer and the function returns true
	On failure, if hide_errors_from_user is set to false an error is displayed to the user and false is returned, if the number of bytes read doesn't match the requested amount ERROR_HANDLE_EOF is set
	*/
	bool read(file_reference *filo_ptr, LPVOID data_buffer, DWORD nNumberOfBytesToRead, bool hide_errors_from_user);

	/* Returns success */
	bool write(file_reference *filo_ptr, LPVOID data, size_t data_size);

	/* Returns sucess, last_write_time is set to the file modification data if sucessful otherwise it's not modified */
	bool get_last_write(file_reference *file_reference, FILETIME *last_write_time);

	/* Returns success */
	bool get_size(file_reference *data, DWORD *file_size);

	/* Can be used to truncate or extend an open file , returns success */
	inline bool change_size(file_reference *filo_ptr, LONG new_size)
	{
		if (LOG_CHECK(filo_ptr->handle))
		{
			if (SetFilePointer(filo_ptr->handle, new_size, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
			{
				return SetEndOfFile(filo_ptr->handle);
			}
		}
		else {
			SetLastError(ERROR_INVALID_HANDLE);
		}
		return false;
	}

	std::string get_path_info(file_reference *data, PATH_FLAGS flags);

	/* reads a file into memory, free with delete[] */
	inline void *read_into_memory(file_reference *file, DWORD &size)
	{
		if (!open(file, FILO_OPEN_READ))
		{
			size = NONE;
			return nullptr;
		}

		void *output = nullptr;
		size = get_eof(file);
		if (size && LOG_CHECK(size != INVALID_FILE_SIZE))
		{
			output = new char[size];
			if (!LOG_CHECK(read(file, output, size, 0)))
			{
				delete[] output;
				output = nullptr;
			}
		}
		close(file);
		return output;
	}
};

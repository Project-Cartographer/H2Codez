#pragma once
#include <string>
#include <unordered_set>
#include <filesystem>
namespace fs = std::filesystem;

inline void find_all_files_with_extension(std::unordered_set<std::string> &files, const char* folder, const char* ext)
{
	;
	std::string file_ext_full = std::string(".") + ext;
	for (auto& p : fs::recursive_directory_iterator(folder))
	{
		std::string ext = p.path().extension().string();
		if (ext == file_ext_full)
		{
			std::string bitmap_path = p.path().parent_path().generic_string() + "\\" + p.path().stem().generic_string();
			std::replace(bitmap_path.begin(), bitmap_path.end(), '/', '\\');
			files.insert(bitmap_path);
		}
	}
};


inline void find_all_files_with_extension(std::unordered_set<std::string>& files, const std::string& folder, const std::string& ext)
{
	find_all_files_with_extension(files, folder.c_str(), ext.c_str());
};

inline void find_all_files_with_extension(std::unordered_set<std::string>& files, const std::string& folder, const char* ext)
{
	find_all_files_with_extension(files, folder.c_str(), ext);
};

inline void find_all_files_with_extension(std::unordered_set<std::string>& files, const char* folder, const std::string& ext)
{
	find_all_files_with_extension(files, folder, ext.c_str());
};

inline std::unordered_set<std::string> find_all_files_with_extension(const char* folder, const char* ext)
{
	std::unordered_set<std::string> files;
	find_all_files_with_extension(files, folder, ext);
	return files;
};


inline std::unordered_set<std::string> find_all_files_with_extension(const std::string& folder, const std::string& ext)
{
	return find_all_files_with_extension(folder.c_str(), ext.c_str());
};

inline std::unordered_set<std::string> find_all_files_with_extension(const std::string& folder, const char* ext)
{
	return find_all_files_with_extension(folder.c_str(), ext);
};

inline std::unordered_set<std::string> find_all_files_with_extension(const char* folder, const std::string& ext)
{
	return find_all_files_with_extension(folder, ext.c_str());
};

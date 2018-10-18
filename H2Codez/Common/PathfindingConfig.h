#pragma once
#include <unordered_set>
#include "util/string_util.h"

class pathfinding_config
{
public:
	/* Reads in settings from a file */
	bool parse_file(std::ifstream &file)
	{
		enum mode
		{
			undefined,
			keep,
			remove
		};
		mode current_mode = undefined;
		while (file)
		{
			std::string line;
			std::getline(file, line);
			str_trim(line);
			line = tolower(line);

			if (line == get_keep_header())
			{
				current_mode = keep;
				continue;
			}
			if (line == get_remove_header())
			{
				current_mode = remove;
				continue;
			}

			if (current_mode == undefined)
				continue;

			unsigned short surface_idx = NONE;
			try {
				surface_idx = static_cast<unsigned short>(std::stoul(line));
			}
			catch (...) {
				continue;
			}
			if (LOG_CHECK(current_mode != undefined))
			{
				switch (current_mode)
				{
				case remove:
					surfaces_to_remove.insert(surface_idx);
				case keep:
					surfaces_to_keep.insert(surface_idx);
				}
			}
			else {
				return false;
			}
		}
		return !file.bad();
	}

	/* Reads in settings from a file */
	bool parse_file(const std::string &file_name)
	{
		std::ifstream file(file_name);
		if (file)
		{
			return parse_file(file);
		}
		return false;
	}

	bool write_to_file(std::ofstream &file)
	{
		if (file)
		{
			std::unordered_set<unsigned short> surfaces_kept;
			for (const auto surface : surfaces_to_keep)
			{
				if (surfaces_to_remove.count(surface) == 0)
					surfaces_kept.insert(surface);
			}

			if (surfaces_kept.size() > 0)
			{
				file << get_keep_header() << std::endl;
				for (const size_t surface : surfaces_kept)
					file << surface << std::endl;
				file << std::endl;
			}

			if (surfaces_to_remove.size() > 0)
			{
				file << get_remove_header() << std::endl;
				for (const size_t surface : surfaces_to_remove)
					file << surface << std::endl;
				file << std::endl;
			}
		}
		return !file.bad();
	}

	bool write_to_file(const std::string &file_name)
	{
		std::ofstream file(file_name);
		if (file)
		{
			return write_to_file(file);
		}
		return false;
	}

	pathfinding_config(const std::string &file_name)
	{
		parse_file(file_name);
	}
	pathfinding_config() {};

	/* Should include surface even if other checks fail */
	bool should_force_keep_surface(unsigned short surface)
	{
		return should_keep_surface(surface) && surfaces_to_keep.count(surface) > 0;
	}

	/* Surface **not** marked for removal */
	bool should_keep_surface(unsigned short surface)
	{
		return !should_remove_surface(surface);
	}

	/* Surface marked for removal */
	bool should_remove_surface(unsigned short surface)
	{
		return surfaces_to_remove.count(surface) > 0;
	}

	/* Mark a surface as removed, overrides keep_surface */
	void remove_surface(unsigned short surface)
	{
		surfaces_to_remove.insert(surface);
	}

	/* Mark a surface as included, overriden by remove_surface */
	void keep_surface(unsigned short surface)
	{
		surfaces_to_keep.insert(surface);
	}

private:

	const std::string &get_remove_header() const
	{
		const static std::string remove_header = "[remove]";
		return remove_header;
	}

	const std::string &get_keep_header() const
	{
		const static std::string keep_header = "[keep]";
		return keep_header;
	}

	std::unordered_set<unsigned short> surfaces_to_remove;
	std::unordered_set<unsigned short> surfaces_to_keep;
};
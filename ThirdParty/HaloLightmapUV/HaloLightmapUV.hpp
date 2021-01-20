/*
The MIT License (MIT)

Copyright (c) 2021 num0005 <double.null@outlook.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <locale>
#include <stdexcept>

namespace HaloLightmapUV {
	
	// type used for all float point numbers
	typedef float real;
	
	struct Position3d {
		real x;
		real y;
		real z;
	};
	
	struct Position2d {
		real x;
		real y;
	};
	
	struct Vertex {
		Position3d location;
		Position2d texture_coord;
	};
	
	struct Entry {
		std::string name;
		std::vector<Vertex> vertices;
	};

	class File {
		std::vector<Entry> _entries;
		std::locale _locale = std::locale("C");

		/// <summary>
		/// Helper function for reading a quoted string. Throws an exception if it can't read the string correctly
		/// </summary>
		/// <param name="stream">The stream to read from</param>
		/// <returns>The parsed string</returns>
		std::string ReadQuotedString(std::ifstream& stream) {
			std::string parsed_string;

			char character;
			stream >> character;
			if (character != '"')
				throw std::runtime_error("String doesn't start with quotation mark");

			while (true) {
				stream >> character;
				if (character == '"')
					break;
				if (std::isprint(character, _locale))
					parsed_string += character;
				else
					throw std::runtime_error("String contains unprintable characters");
			}

			return parsed_string;
		}
		
		void Read(const std::string &path) {
			_entries.clear();
			
			std::ifstream ifs;
			ifs.exceptions(std::ifstream::failbit);
			ifs.imbue(_locale);

			ifs.open(path, std::ios::in);
			
			unsigned int entry_count;
			ifs >> entry_count;
			while (entry_count--) {
				Entry entry;
				entry.name = ReadQuotedString(ifs);
				
				unsigned int vert_count;
				ifs >> vert_count;
				while (vert_count--) {
					Vertex vert;
					ifs >> vert.location.x >> vert.location.y >> vert.location.z;
					ifs >> vert.texture_coord.x >> vert.texture_coord.y;
					entry.vertices.push_back(vert);
				}
				_entries.push_back(entry);
			}
		}
		
	public:
		File() = default;
		// Construct from a file
		File(const std::string &path) {
			Read(path);
		}
		
		// Write the .LUV file to disk
		void Write(const std::string &path) {
			std::ofstream ofs(path);
			ofs.imbue(_locale);
			
			ofs << _entries.size() << std::endl;
			for (const Entry &entry : _entries ) {
				ofs << '"' << entry.name << '"' << std::endl << entry.vertices.size() << std::endl;
				for (const Vertex &vert: entry.vertices) {
					ofs << "\t" << vert.location.x << "\t" << vert.location.y << "\t" << vert.location.z << std::endl;
					ofs << "\t" << vert.texture_coord.x << "\t" << vert.texture_coord.y << std::endl;
				}
			}
		}
		
		inline std::vector<Entry> &GetEntries() noexcept { return _entries; };
	};
}

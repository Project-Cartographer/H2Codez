#pragma once
#include <string>
#include "Common\BlamBaseTypes.h"

namespace TagDumper
{
	bool dump_as_xml(datum tag, const std::string &xml_dump_name);
}

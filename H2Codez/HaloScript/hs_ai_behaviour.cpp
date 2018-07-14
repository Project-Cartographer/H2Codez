#include "../stdafx.h"
#include "hs_ai_behaviour.h"

std::string search_for;
bool matching_string(std::pair<const HaloScriptCommon::ai_behaviour, std::string> i) {
	return i.second == search_for;
}

HaloScriptCommon::ai_behaviour HaloScriptCommon::string_to_ai_behaviour(const std::string &contents)
{
	search_for = contents;
	auto i = std::find_if(ai_behaviour_string.begin(), ai_behaviour_string.end(), matching_string);
	if (i == ai_behaviour_string.end()) {
		return ai_behaviour::invalid;
	}
	else {
		return i->first;
	}
}

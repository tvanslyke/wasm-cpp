#ifndef FUNCTION_ANALYSIS_H
#define FUNCTION_ANALYSIS_H

#include <string>
#include "WasmInstruction.h"

struct StackSnapshot {
	
	
	
};

template <class It>
std::basic_string<LanguageType> get_snapshot(It first, It snap_pos, It last)
{
	std::basic_string<LanguageType> stack;
	for(auto pos = first; pos != snap_pos;)
	{
		
	}
}

#endif /* FUNCTION_ANALYSIS_H */

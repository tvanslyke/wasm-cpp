#ifndef PARSE_WASM_FUNCTION_DEF_H
#define PARSE_WASM_FUNCTION_DEF_H
#include "wasm_base.h"
#include "parse/leb128/leb128.h"
#include <string>
#include <vector>
#include <algorithm>
#include "wasm_instruction.h"

using opcode_t = std::underlying_type_t<wasm_instruction>;

struct wasm_signature
{
	
};

struct wasm_function_def
{

	std::basic_string<opcode_t> code;
};



#endif /* PARSE_WASM_FUNCTION_DEF_H */ 

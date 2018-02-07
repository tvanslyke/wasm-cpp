#ifndef FUNCTION_WASM_FUNCTION_SPACE
#define FUNCTION_WASM_FUNCTION_SPACE

#include "function/wasm_function.h"
#include "wasm_instruction_types.h"
#include <vector>
struct wasm_function_space
{
	

private:
	std::vector<wasm_function> function_defs;
};


#endif /* FUNCTION_WASM_FUNCTION_SPACE */

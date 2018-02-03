#ifndef WASM_FUNCTIONS_H
#define WASM_FUNCTIONS_H

#include <vector>
#include "wasm_base.h"
#include "wasm_function.h"
#include "wasm_instruction_types.h"
#include "parse/wasm_module_def.h"

struct wasm_function_def
{
	wasm_code_string_t code;
	func_sig_id_t signature;
	std::size_t parameter_count;
	std::size_t return_count;
	std::size_t locals_count;
};

struct wasm_functions
{
	wasm_function& get_function(std::size_t index);
	std::size_t add_function_definition(wasm_function_def);
private:
	std::vector<wasm_function> functions;
};


#endif /* WASM_FUNCTIONS_H */

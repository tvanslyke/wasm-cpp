#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "parse/wasm_module_def.h"
#include "function/wasm_function.h"
#include <vector>

struct wasm_module
{
	const std::string name;
	const std::vector<func_sig_id_t> types;
	std::vector<wasm_function*> functions;
	std::vector<std::ptrdiff_t> memory_offsets;
	std::vector<std::ptrdiff_t> global_variable_offsets;
};

#endif /* MODULE_WASM_MODULE_H */

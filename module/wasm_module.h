#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "parse/wasm_module_def.h"
#include "function/wasm_function.h"
#include <vector>

struct wasm_module
{


	const wasm_function* function_at(std::size_t index) const;
	const wasm_linear_memory* memory_at(std::size_t index) const;
	wasm_linear_memory* memory_at(std::size_t index);
	wasm_value_t& global_at(std::size_t index);
	const std::string name;
	const std::vector<func_sig_id_t> types;
private:
	std::vector<const wasm_function*> functions;
	std::vector<std::ptrdiff_t> memory_offsets;
	std::vector<std::ptrdiff_t> global_variable_offsets;
};

#endif /* MODULE_WASM_MODULE_H */

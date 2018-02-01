#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "parse/wasm_module_def.h"
#include "function/wasm_function.h"

struct wasm_module
{
	const std::vector<wasm_function_signature_handle> types;
	const std::vector<wasm_function> functions;
	std::vector<wasm_linear_memory> memories;
	std::vector<wasm_value_t> globals;
	const std::vector<bool> global_mutabilities;
};


template <class ParseFunc>
void 
wasm_module instantiate_wasm_module(wasm_module_def& def)
{
	std::vector<wasm_function_signature_handle> func_sigs;
	std::vector<wasm_function> func_defs;
	std::vector<wasm_linear_memory> memories;
	std::vector<wasm_value_t> globals;
	std::vector<bool> global_mutabilities;
}




#endif /* MODULE_WASM_MODULE_H */

#ifndef PARSE_DEFS_FUNCTION_DEFS_H
#define PARSE_DEFS_FUNCTION_DEFS_H
#include "wasm_base.h"
#include "function/wasm_function.h"


struct wasm_function_defs
{
	void add_module(const wasm_module_def& module);
private:
	std::vector<wasm_function> function_defs;
	FunctionSignatureRegistrar sig_registrar;
};

#endif /* PARSE_DEFS_FUNCTION_DEFS_H */

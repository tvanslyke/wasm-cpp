#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "parse/wasm_module_def.h"
#include "function/wasm_function.h"


struct wasm_module
{
	

private:
	std::vector<wasm_function_signature> types;
	std::vector<wasm_function> functions;
};

#endif /* MODULE_WASM_MODULE_H */

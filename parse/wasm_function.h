#ifndef FUNCTION_WASM_FUNCTION_H
#define FUNCTION_WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_types.h"


struct wasm_function_signature
{
	const wasm_type_code form{wasm_func_v};
	wasm_uint32_t param_count;
	wasm_param
};
struct wasm_function
{

	wasm_uint32_t body_size;
	

};

#endif /* WASM_FUNCTION_H */

#ifndef WASM_FUNCTION_H
#define WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include "wasm_value.h"
#include <vector>
#include <memory>


struct wasm_function_signature
{
	using char_type = std::underlying_type_t<wasm_language_type>;
	// std::basic_string to leverage SSO
	const std::size_t param_count;
	const std::basic_string<char_type> types;
};

std::shared_ptr<wasm_function_signature> 
make_wasm_function_signature

template <class It

struct wasm_function
{
	using char_type = std::underlying_type_t<wasm_value_t>;
	std::shared_ptr<wasm_function_signature> signature;
	std::basic_string<
};




#endif /* WASM_FUNCTION_H */


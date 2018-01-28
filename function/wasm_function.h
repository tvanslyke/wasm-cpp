#ifndef WASM_FUNCTION_H
#define WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include "wasm_value.h"
#include <vector>


struct wasm_function
{

	wasm_function(std::size_t local_count, std::size_t instruction_count, wasm_instruction_t* instrs);
	const wasm_instruction* get_code() const;
	std::size_t locals_count() const;
	std::size_t instruction_count() const;
private:
	struct wasm_function_impl;
	struct wasm_function_deleter;
	using wasm_function_impl_ptr = std::unique_ptr<wasm_function_impl, wasm_function_deleter>;

	wasm_function_impl_ptr impl;
};








#endif /* WASM_FUNCTION_H */


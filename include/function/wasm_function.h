#ifndef FUNCTION_WASM_FUNCTION_H
#define FUNCTION_WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include "wasm_value.h"
#include <memory>
#include <string>

struct wasm_function_storage;
struct wasm_function
{
	using opcode_t = wasm_opcode::wasm_opcode_t;
	using code_string_t = std::basic_string<opcode_t>;
	wasm_function(const opcode_t* code_begin, const opcode_t* code_end, std::size_t sig, 
		std::size_t nlocals, std::size_t nparams, std::size_t nreturns);
	const opcode_t* code() const;
	std::size_t code_size() const;
	std::size_t signature() const;
	std::size_t locals_count() const;
	std::size_t param_count() const;
	std::size_t return_count() const;
private:
	struct WasmFunctionDeleter {
		void operator()(const void* func_storage) const;
	};
	using storage_ptr_t = std::unique_ptr<const wasm_function_storage, WasmFunctionDeleter>;
	storage_ptr_t func_storage;
	const wasm_function_storage* get() const
	{ return func_storage.get(); }
	friend class wasm_call_stack;
};


#endif /* FUNCTION_WASM_FUNCTION_H */

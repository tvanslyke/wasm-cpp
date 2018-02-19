#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "function/wasm_function.h"
#include "module/wasm_table.h"
#include "module/wasm_linear_memory.h"
#include <vector>

struct wasm_program_def;

struct wasm_program_state
{
	wasm_program_state(
		std::vector<wasm_function>&& functions_,
		std::vector<wasm_table>&& tables_,
		std::vector<wasm_linear_memory>&& memories_,
		std::vector<wasm_value_t>&& globals_,
		std::vector<bool>&& global_mutabilities_,
		std::size_t start_function_
	);

	wasm_program_state();
	const wasm_function& function_at(std::size_t index) const;

	const wasm_function& table_function_at(std::size_t index) const;
	
	const wasm_linear_memory& const_memory_at(std::size_t index) const;
	const wasm_linear_memory& memory_at(std::size_t index) const;
	wasm_linear_memory& memory_at(std::size_t index);

	const wasm_value_t& const_global_at(std::size_t index) const;
	const wasm_value_t& global_at(std::size_t index) const;
	wasm_value_t& global_at(std::size_t index);

private:
	static std::vector<wasm_function> init_functions(wasm_program_def& program_def);
	static std::vector<wasm_linear_memory> init_memories(wasm_program_def& program_def);
	static std::vector<wasm_table> init_tables(wasm_program_def& program_def);
	static std::vector<wasm_value_t> init_globals(wasm_program_def& program_def);
	static std::vector<bool> init_global_mutabilities(wasm_program_def& program_def);

	// TODO: segregate const globals and non-const globals
	const std::vector<wasm_function> functions;
	std::vector<wasm_linear_memory> memories;
	const std::vector<wasm_table> tables;
	std::vector<wasm_value_t> globals;
	const std::vector<bool> global_mutabilities;
public:
	const wasm_function& start_function;
};

#endif /* MODULE_WASM_MODULE_H */

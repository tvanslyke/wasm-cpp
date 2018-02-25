#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "function/WasmFunction.h"
#include "module/wasm_table.h"
#include "module/wasm_linear_memory.h"
#include <vector>
#include <unordered_map>
#include <string>

struct wasm_program_def;

struct wasm_program_state
{
	using name_map_t = std::array<std::unordered_map<wasm_uint32_t, std::string>, 4>;
	wasm_program_state(
		std::vector<WasmFunction>&& functions_,
		std::vector<wasm_table>&& tables_,
		std::vector<wasm_linear_memory>&& memories_,
		std::vector<wasm_value_t>&& globals_,
		std::vector<bool>&& global_mutabilities_,
		name_map_t&& name_map_,
		std::size_t start_function_
	);

	wasm_program_state();
	const WasmFunction& function_at(std::size_t index) const;

	const WasmFunction& table_function_at(std::size_t index) const;
	std::size_t table_function_index(std::size_t index) const;
	
	const wasm_linear_memory& const_memory_at(std::size_t index) const;
	const wasm_linear_memory& memory_at(std::size_t index) const;
	wasm_linear_memory& memory_at(std::size_t index);

	const wasm_value_t& const_global_at(std::size_t index) const;
	const wasm_value_t& global_at(std::size_t index) const;
	wasm_value_t& global_at(std::size_t index);
private:
	static std::vector<WasmFunction> init_functions(wasm_program_def& program_def);
	static std::vector<wasm_linear_memory> init_memories(wasm_program_def& program_def);
	static std::vector<wasm_table> init_tables(wasm_program_def& program_def);
	static std::vector<wasm_value_t> init_globals(wasm_program_def& program_def);
	static std::vector<bool> init_global_mutabilities(wasm_program_def& program_def);

	// TODO: segregate const globals and non-const globals
	const std::vector<WasmFunction> functions;
	std::vector<wasm_linear_memory> memories;
	const std::vector<wasm_table> tables;
	std::vector<wasm_value_t> globals;
	const std::vector<bool> global_mutabilities;
	// for debugging, stack traces, etc
public:
	const name_map_t name_map;
	const std::size_t start_function_index;
	const WasmFunction& start_function;
	friend int main(int, const char* argv[]);
};

#endif /* MODULE_WASM_MODULE_H */

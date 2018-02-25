#include "module/wasm_program_state.h"

wasm_program_state::wasm_program_state(
		std::vector<WasmFunction>&& functions_,
		std::vector<wasm_table>&& tables_,
		std::vector<wasm_linear_memory>&& memories_,
		std::vector<wasm_value_t>&& globals_,
		std::vector<bool>&& global_mutabilities_,
		name_map_t&& name_map_,
		std::size_t start_function_):
	functions(std::move(functions_)),
	memories(std::move(memories_)),
	tables(std::move(tables_)),
	globals(std::move(globals_)),
	global_mutabilities(std::move(global_mutabilities_)),
	name_map(std::move(name_map_)),
	start_function_index(start_function_),
	start_function(functions.at(start_function_index))
{

}

const WasmFunction& wasm_program_state::function_at(std::size_t index) const
{
	return functions[index];
}

const WasmFunction& wasm_program_state::table_function_at(std::size_t index) const
{
	return functions[tables[0].access_indirect(index)];
}
	
std::size_t wasm_program_state::table_function_index(std::size_t index) const
{
	return tables[0].access_indirect(index);
}

const wasm_linear_memory& wasm_program_state::const_memory_at(std::size_t index) const
{
	return memories[index];
}

const wasm_linear_memory& wasm_program_state::memory_at(std::size_t index) const
{
	return const_memory_at(index);
}
wasm_linear_memory& wasm_program_state::memory_at(std::size_t index)
{
	return memories[index];
}

const wasm_value_t& wasm_program_state::const_global_at(std::size_t index) const
{
	return globals[index];
}
const wasm_value_t& wasm_program_state::global_at(std::size_t index) const
{
	return const_global_at(index);
}
wasm_value_t& wasm_program_state::global_at(std::size_t index)
{
	assert(global_mutabilities[index]);
	return globals[index];
}


#include "parse/wasm_program_def.h"
#include "module/wasm_program_state.h"

// using wasm_program_def::function_def_t;
// using wasm_program_def::global_variable_def_t;
// using wasm_program_def::linear_memory_def_t;
// using wasm_program_def::table_def_t;
// using wasm_program_def::function_def_t;

std::vector<wasm_function> wasm_program_state::init_functions(wasm_program_def& program_def)
{
	std::vector<wasm_function> funcs;
	funcs.reserve(program_def.function_defs.size());
	for(const auto& funcdef: program_def.function_defs)
	{
		auto& def = funcdef.value();
		funcs.emplace_back(def.code, def.sig, def.param_count, def.locals_count, def.return_count);
	}
	return funcs;
}
std::vector<wasm_linear_memory> wasm_program_state::init_memories(wasm_program_def& program_def)
{
	std::vector<wasm_linear_memory> mems;
	mems.reserve(program_def.linear_memory_defs.size());
	for(auto& memdef: program_def.linear_memory_defs)
	{
		auto& def = memdef.value();
		assert(def.initial_size == def.initial_memory.size());
		mems.emplace_back(std::move(def.initial_memory), std::move(def.maximum));
	}
	return mems;
}

std::vector<wasm_table> wasm_program_state::init_tables(wasm_program_def& program_def)
{
	std::vector<wasm_table> tables;
	tables.reserve(program_def.table_defs.size());
	for(auto& tabledef: program_def.table_defs)
	{
		auto& def = tabledef.value();
		assert(def.initial_size == def.functions.size());
		tables.emplace_back(std::move(def.functions), def.element_type, def.maximum);
	}
	return tables;
}

std::vector<wasm_value_t> wasm_program_state::init_globals(wasm_program_def& program_def)
{
	std::vector<wasm_value_t> glbls;
	glbls.reserve(program_def.global_variable_defs.size());
	for(auto& glbldef: program_def.global_variable_defs)
	{
		auto& def = glbldef.value();
		auto& value = def.initial_value;
		if(value.has_value())
		{
			glbls.push_back(def.initial_value.value());
		}
		else
		{
			std::cerr << "WARNING: Uninitialized global found during program startup." << std::endl;
			glbls.emplace_back();
		}
	}
	return glbls;
}
std::vector<bool> wasm_program_state::init_global_mutabilities(wasm_program_def& program_def)
{
	std::vector<bool> muts;
	muts.reserve(program_def.global_variable_defs.size());
	for(auto& glbldef: program_def.global_variable_defs)
	{
		auto& def = glbldef.value();
		muts.push_back(def.mutability);
	}
	return muts;
}
wasm_program_state::wasm_program_state(wasm_program_def&& program_def):
	functions(init_functions(program_def)),
	memories(init_memories(program_def)),
	tables(init_tables(program_def)),
	globals(init_globals(program_def)),
	global_mutabilities(init_global_mutabilities(program_def)),
	start_function(&(functions.at(program_def.program_start_function)))
{
	
}
const wasm_function& wasm_program_state::function_at(std::size_t index) const
{
	return functions[index];
}

const wasm_function& wasm_program_state::table_function_at(std::size_t index) const
{
	return functions[tables[0].access_indirect(index)];
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


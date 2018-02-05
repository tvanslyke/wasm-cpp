#ifndef MODULE_WASM_MODULE_H
#define MODULE_WASM_MODULE_H

#include "parse/defs/program_def.h"
#include "function/wasm_function.h"
#include <vector>

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
		funcs.emplace_back(funcdef.code, funcdef.sig, funcdef.param_count, funcdef.locals_cound);
	return funcs;
}
std::vector<wasm_linear_memory> wasm_program_state::init_memories(wasm_program_def& program_def)
{
	std::vector<wasm_linear_memory> mems;
	mems.reserve(program_def.linear_memory_defs.size());
	for(auto& memdef: program_def.linear_memory_defs)
	{
		assert(memdef.initial_size == memdef.initial_memory.size());
		mems.emplace_back(std::move(memdef.initial_memory), std::move(memdef.maximum));
	}
	return mems;
}

std::vector<wasm_table> wasm_program_state::init_tables(wasm_program_def& program_def)
{
	std::vector<wasm_table> tables;
	tables.reserve(program_def.table_defs.size());
	for(auto& tabledef: program_def.table_defs)
	{
		assert(tabledef.initial_size == tabledef.functions.size());
		tables.emplace_back(std::move(tabledef.functions), tabledef.element_type, tabledef.maximum);
	}
	return table;
}

std::vector<wasm_value_t> wasm_program_state::init_globals(wasm_program_def& program_def)
{
	std::vector<bool> glbls;
	glbls.reserve(program_def.global_variable_defs.size());
	for(auto& glbldef: program_def.global_variable_defs)
	{
		auto& value = glbldef.initial_value;
		if(value.has_value())
		{
			glbls.push_back(glbldef.initial_value.value());
		}
		else
		{
			std::cerr << "WARNING: Uninitialized global found during program startup." << std::endl;
			glbls.emplace_back();
		}
	}
	return glbls;
}
std::vector<bool> init_global_mutabilities(wasm_program_def& program_def)
{
	std::vector<bool> muts;
	muts.reserve(program_def.global_variable_defs.size());
	for(auto& glbldef: program_def.global_variable_defs)
		muts.push_back(glbldef.mutability);
	return muts;
}
wasm_program_state::wasm_program_state(wasm_program_def&& program_def):
	functions(init_functions(program_def)),
	functions(init_memories(program_def)),
	functions(init_tables(program_def)),
	functions(init_globals(program_def)),
	functions(init_global_mutabilities(program_def)),
	start_function(program_def.program_start_function)
{
	assert(program_start_function > -1);
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


#endif /* MODULE_WASM_MODULE_H */

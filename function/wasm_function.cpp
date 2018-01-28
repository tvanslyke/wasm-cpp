#include "wasm_function.h"
#include <cstddef>
#include <type_traits>

struct wasm_function::wasm_function_impl
{
	wasm_function_impl(std::size_t local_count_, std::size_t instruction_count_) noexcept:
		local_count(local_count_), instruction_count(instruction_count_) 
	{
		
	}

	const std::size_t local_count;
	const std::size_t instruction_count;
	const wasm_instruction code[1];
};

static_assert(std::is_trivially_destructible_v<wasm_function_impl>);
static_assert(std::is_standard_layout_v<wasm_function_impl>);

struct wasm_function::wasm_function_deleter
{
	void operator()(void* p) const
	{
		std::free(p);
	}
};

wasm_function_impl_ptr
make_wasm_function_impl(std::size_t local_count, std::size_t instruction_count, wasm_instruction_t* instrs)
{
	constexpr std::size_t base_size = offsetof(wasm_function_impl, buff);
	std::size_t code_size = instruction_count * sizeof(wasm_instruction);
	wasm_function_impl* func = 
		(wasm_function_impl*)std::malloc(base_size + code_size);
	if(not func_mem)
		throw std::bad_alloc("Allocation failed for wasm_function_impl object");
	wasm_instruction* code = reinterpret_cast<wasm_instruction*>(
		reinterpret_cast<char*>(func_mem) + base_size
	);
	std::memcpy(code, instrs, instruction_count * sizeof(wasm_instruction));
	new(func) wasm_func_impl(local_count, instruction_count);
	return wasm_function_impl_ptr(func);
}



wasm_function::wasm_function(std::size_t local_count, 
			     std::size_t instruction_count, 
			     wasm_instruction_t* instrs):
	impl(make_wasm_function_impl(local_count, instruction_count, instrs)
{

}

const wasm_instruction* wasm_function::get_code() const
{
	return impl->code;
}

std::size_t wasm_function::locals_count() const
{
	return impl->local_count;
}

std::size_t wasm_function::instruction_count() const
{
	return impl->instruction_count;
}



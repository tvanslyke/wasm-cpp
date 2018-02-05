#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include "wasm_base.h"
#include "wasm_value.h"
#include "function/wasm_function.h"
#include "module/wasm_resizable_limits.h"


struct wasm_table
{
	wasm_table() = delete;
	wasm_table(std::vector<std::size_t>&& offsets, wasm_language_type tp, std::optional<std::size_t> maxm): 
		function_offsets(std::move(offsets)), type(tp), 
		maximum_size(maxm.has_value() ? maxm.value() : function_offsets.max_size())
	{
		assert(type == wasm_language_type::anyfunc);
	}

	std::size_t access_indirect(std::size_t index) const
	{
		return function_offsets[index];
	}
private:
	std::vector<std::size_t> function_offsets;
	const wasm_language_type type;
	const std::size_t maximum_size;
};


#endif /* MODULE_WASM_TABLE_H */

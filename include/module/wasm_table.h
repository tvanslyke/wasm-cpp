#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include <cstddef>
#include <vector>
#include <optional>
#include "wasm_base.h"
#include "wasm_value.h"
#include "function/wasm_function.h"

struct wasm_table
{
	using typecode_t = std::underlying_type_t<wasm_language_type>;
	wasm_table() = delete;
	wasm_table(std::vector<std::size_t>&& offsets, typecode_t tp, std::optional<std::size_t> maxm): 
		function_offsets(std::move(offsets)), type(tp), 
		maximum_size(maxm.has_value() ? maxm.value() : function_offsets.max_size())
	{
		assert(type == (typecode_t)wasm_language_type::anyfunc);
	}

	std::size_t access_indirect(std::size_t index) const
	{
		return function_offsets[index];
	}
private:
	std::vector<std::size_t> function_offsets;
	const typecode_t type;
	const std::size_t maximum_size;
};


#endif /* MODULE_WASM_TABLE_H */

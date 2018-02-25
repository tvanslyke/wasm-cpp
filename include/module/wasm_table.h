#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include <cstddef>
#include <vector>
#include <optional>
#include "wasm_base.h"
#include "wasm_value.h"
#include "function/wasm_function.h"
#include "module/cfunc_wrapper.h"



struct CFunc
{
	const std::function<wasm_value_t* (wasm_value_t*, wasm_value_t*)> function;
	const std::size_t sig;
};




struct wasm_table
{
	using typecode_t = std::underlying_type_t<wasm_language_type>;
	using value_type = std::variant<std::monostate, std::size_t, CFunc>;
	wasm_table() = delete;
	wasm_table(std::vector<std::size_t>&& offsets, typecode_t tp, std::optional<std::size_t> maxm): 
		function_offsets(std::move(offsets)), type(tp), 
		maximum_size(maxm.has_value() ? maxm.value() : function_offsets.max_size())
	{
		assert(type == (typecode_t)wasm_language_type::anyfunc);
	}

	std::size_t get_wasm_function_index(std::size_t index) const
	{
		return std::get<std::size_t>(functions.at(index));
	}
	
	CFunc& get_c_function(std::size_t index) const
	{
		return std::get<CFunc>(functions.at(index));
	}
	
	bool is_wasm_function(std::ptrdiff_t index) const
	{
		return std::holds_alternative<std::size_t>(functions.at(index));
	}

	bool is_c_function(std::ptrdiff_t index) const
	{
		return std::holds_alternative<CFunc>(functions.at(index));
	}

	bool is_null(std::ptrdiff_t index) const
	{
		return std::holds_alternative<std::monostate>(functions.at(index));
	}

private:
	std::vector<value_type> functions;
	const typecode_t type;
	const std::size_t maximum_size;
};




#endif /* MODULE_WASM_TABLE_H */

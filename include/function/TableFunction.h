#ifndef FUNCTION_TABLE_FUNCTION_H
#define FUNCTION_TABLE_FUNCTION_H

#include "wasm_base.h"
#include "wasm_value.h"


namespace wasm {

struct TableFunction:
	private std::variant<
		const WasmFunction* const*,
		std::function<void(gsl::span<const WasmValue>, gsl::span<WasmValue>)>
	>
{
	
	using args_type = gsl::span<const WasmValue>;
	using result_type = gsl::span<WasmValue>;
	using wasm_function_type = const WasmFunction* const;
	using c_function_type =	std::function<void(args_type, result_type)>;
	using variant_type = std::variant<wasm_function_type*, c_function_type>;

	using variant_type::variant_type;

	TableFunction() = delete;

	TableFunction(const TableFunction&) = default;
	TableFunction(TableFunction&&) = default;
		
	TableFunction& operator=(const TableFunction&) = default;
	TableFunction& operator=(TableFunction&&) = default;

	bool holds_wasm_function() const
	{ return std::holds_alternative<wasm_function_type*>(as_variant()); }

	bool holds_c_function() const
	{ return std::holds_alternative<c_function_type>(as_variant()); }

	wasm_function_type get_wasm_function() const
	{
		assert(holds_wasm_function());
		return *std::get<wasm_function_type*>(as_variant());
	}

	const c_function_type& get_c_function() const
	{
		assert(holds_c_function());
		return std::get<c_function_type>(as_variant());
	}

	c_function_type& get_c_function()
	{
		assert(holds_c_function());
		return std::get<c_function_type>(as_variant());
	}

private:
	variant_type& as_variant()
	{ return static_cast<variant_type&>(*this); }

	const variant_type& as_variant() const
	{ return static_cast<const variant_type&>(*this); }

};

} /* namespace wasm */

#endif /* FUNCTION_TABLE_FUNCTION_H */

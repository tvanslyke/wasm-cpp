#ifndef FUNCTION_ANY_FUNCTION_H
#define FUNCTION_ANY_FUNCTION_H

#include "wasm_base.h"
#include "wasm_value.h"
#include "function/CFunction.h"
#include <gsl/gsl>


namespace wasm {

struct AnyFunction:
	private std::variant<
		std::monostate,
		gsl::not_null<WasmFunction* const*>,
		CFunction
	>
{
	using null_function_alt = std::monostate;
	using wasm_function_alt = gsl::not_null<WasmFunction* const*>;
	using c_function_alt = CFunction;
	using variant_type = std::variant<
		null_function_alt,
		wasm_function_alt,
		c_function_alt
	>;

	using variant_type::variant_type;

	AnyFunction() = default;

	AnyFunction(const AnyFunction&) = default;
	AnyFunction(AnyFunction&&) = default;

	AnyFunction& operator=(const AnyFunction&) = default;
	AnyFunction& operator=(AnyFunction&&) = default;

	void emplace_wasm_function(gsl::not_null<wasm_function_alt> func)
	{ as_variant.emplace<wasm_function_alt>(func); }

	void emplace_c_function(CFunction cfunc)
	{ as_variant().emplace<c_function_alt>(std::move(cfunc)); }

	void emplace_null_function()
	{ as_variant().emplace<null_function_type>(std::monostate{}); }

	bool is_wasm_function() const
	{ return std::holds_alternative<wasm_function_alt>(as_variant()); }

	bool is_c_function() const
	{ return std::holds_alternative<c_function_alt>(as_variant()); }

	bool is_null() const
	{ return std::holds_alternative<std::monostate>(as_variant()); }

	const WasmFunction* get_wasm_function() const
	{
		auto f = std::get<wasm_function_alt>(as_variant());
		return *f;
	}

	WasmFunction* get_wasm_function() 
	{ return const_cast<WasmFunction*>(get_wasm_function()); }

	c_function_type get_c_function() const
	{ return std::get<c_function_alt>(as_variant()); }

private:
	variant_type& as_variant()
	{ return static_cast<variant_type&>(*this); }

	const variant_type& as_variant() const
	{ return static_cast<const variant_type&>(*this); }

};


} /* namespace wasm */

#endif /* FUNCTION_ANY_FUNCTION_H */

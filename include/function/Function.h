#ifndef FUNCTION_H
#define FUNCTION_H

#include "function/CFunction.h"
#include "function/WasmFunction.h"

namespace wasm {

struct Function:
	private std::variant<
		gsl::not_null<WasmFunction* const*>,
		CFunction
	>
{
	using null_function_alt = std::monostate;
	using wasm_function_alt = gsl::not_null<WasmFunction* const*>;
	using c_function_alt = CFunction;
	using variant_type = std::variant<
		wasm_function_alt,
		c_function_alt
	>;

	using variant_type::variant_type;

	Function() = default;

	Function(const Function&) = default;
	Function(Function&&) = default;

	Function& operator=(const Function&) = default;
	Function& operator=(Function&&) = default;

	void emplace_wasm_function(gsl::not_null<wasm_function_alt> func)
	{ as_variant.emplace<wasm_function_alt>(func); }

	void emplace_c_function(CFunction cfunc)
	{ as_variant().emplace<c_function_alt>(std::move(cfunc)); }

	bool is_wasm_function() const
	{ return std::holds_alternative<wasm_function_alt>(as_variant()); }

	bool is_c_function() const
	{ return std::holds_alternative<c_function_alt>(as_variant()); }

	const WasmFunction* get_wasm_function() const
	{
		auto f = std::get<wasm_function_alt>(as_variant());
		return *f;
	}

	WasmFunction* get_wasm_function() 
	{ return const_cast<WasmFunction*>(get_wasm_function()); }

	c_function_type get_c_function() const
	{ return std::get<c_function_alt>(as_variant()); }

	const WasmFunctionSignature& signature() const
	{
		return std::visit(
			[](const auto& f) { return f.signature(); },
			as_variant()
		);
	}

	gsl::span<const LanguageType> param_types() const
	{ return param_types(signature()); }

	gsl::span<const LanguageType> return_types() const
	{ return return_types(signature()); }

	std::size_t param_count() const
	{ return param_types().size(); }

	std::size_t return_count() const
	{ return return_types().size(); }

private:
	variant_type& as_variant()
	{ return static_cast<variant_type&>(*this); }

	const variant_type& as_variant() const
	{ return static_cast<const variant_type&>(*this); }

};


struct BadFunctionCall:
	std::bad_function_call
{
	using std::bad_function_call::bad_function_call;
};

struct NullFunctionError:
	BadFunctionCall
{
	using BadFunctionCall::BadFunctionCall;
};

struct BadFunctionSignature:
	BadFunctionCall
{
	using BadFunctionCall::BadFunctionCall;
};
} /* namespace wasm */


#endif /* FUNCTION_H */

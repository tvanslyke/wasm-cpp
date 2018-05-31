#ifndef WASM_FUNCTION_SIGNATURE_H
#define WASM_FUNCTION_SIGNATURE_H

#include "WasmInstruction.h"
#include <unordered_set>
#include <string>
#include <string_view>

struct WasmFunctionSignature
{
	using view_type = std::basic_string_view<LanguageType>;
	using string_type = std::basic_string<LanguageType>;

	WasmFunctionSignature(const parse::FunctionSignature& sig)
		param_count(sig.param_types.size()),
		return_count(sig.return_type ? 1u : 0u),
		data([](){ // immediately-invoked lambda expression
			string_type s;
			if(sig.return_type)
				s.push_back(*sig.return_type);
			s.append(sig.param_types);
			return s;
		}())
	{
		
	}

	friend auto param_types(const WasmFunctionSignature& sig)
	{ return view_type(sig.c_str() + sig.return_count, sig.pram_count); }

	friend view_type return_types(const WasmFunctionSignature& sig)
	{ return view_type(sig.c_str(), sig.return_count); }

private:
	const std::size_t param_count;
	const std::size_t return_count;
	const string_type data;
};

auto return_count(const WasmFunctionSignature& sig)
{ return return_types(sig).size(); }

auto param_count(const WasmFunctionSignature& sig)
{ return param_types(sig).size(); }

bool operator==(const WasmFunctionSignature& left, const WasmFunctionSignature)
{
	return return_count(left) == return_count(right)
		and param_count(left) == param_count(right)
		and return_types(left) == return_types(right) 
		and param_types(left) == param_types(right);
}

bool operator!=(const WasmFunctionSignature& left, const WasmFunctionSignature)
{ return not (left == right); }




std::ostream& operator<<(std::ostream& os, const WasmFunctionSignature& sig)
{
	os << "((param ";
	{
		auto params = param_types(sig);
		if(not params.empty())
		{
			os << params.front();
			params.remove_prefix(1);
			for(auto tp: params)
				os << ' ' << tp;
		}
	}
	os << ") (result ";
	{
		auto returns = return_types(sig);
		if(not returns.empty())
		{
			os << returns.front();
			returns.remove_prefix(1);
			for(auto tp: returns)
				os << ' ' << tp;
		}
	}
	os << "))";
	return os;
}


#endif /* WASM_FUNCTION_SIGNATURE_H */

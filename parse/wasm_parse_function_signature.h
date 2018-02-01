#ifndef PARSE_WASM_PARSE_FUNCTION_SIGNATURE_H
#define PARSE_WASM_PARSE_FUNCTION_SIGNATURE_H

#include "function/wasm_function.h"
#include "parse/leb128/leb128.h"
template <class It>
func_sig_id_t extract_function_signature(
		FunctionSignatureRegistrar& registrar,
		It begin, It end)
{
	using func_param_type_t = std::underlying_type_t<wasm_language_type>;
	
	assert(begin != end);
	if(auto [tp, it] = leb128_decode_sint7(begin, end), begin = it;
		tp != wasm_language_type::func)
	{
		throw std::runtime_error("Attempt to parse WASM function type "
					 "but encountered incorrect 'form' field");
	}
	assert(begin != end);
	
	wasm_uint32_t param_count = 0;
	wasm_uint8_t return_count = 0;
	std::tie(param_count, begin) = leb128_decode_uint32(begin, end);
	assert(begin != end);
	
	std::basic_string<func_param_type_t> sig_string;
	sig_string.reserve(param_count + 1);
	sig_string.resize(param_count);
	for(std::size_t i = 0; i < param_count; ++i)
	{
		std::tie(sig_string[i], begin) = leb128_decode_uint7(begin, end);
		assert(begin != end);
	}
	std::tie(return_count, begin) = leb128_decode_uint1(begin, end);
	if(return_count > 0)
	{
		assert(begin != end);
		// TODO: remove this assert if multiple returns are added to the spec
		assert(return_count == 1);
		sig_string.emplace_back();
		std::tie(sig_string.back(), begin) = leb128_decode_uint7(begin, end);
	}
	return registrar.get_signature(std::move(sig_string), param_count);
}




#endif /* PARSE_WASM_PARSE_FUNCTION_SIGNATURE_H */

#ifndef WASM_FUNCTION_H
#define WASM_FUNCTION_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include "wasm_value.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>


struct wasm_function_signature
{
	using char_type = std::underlying_type_t<wasm_language_type>;
	template <class String>
	wasm_function_signature(std::size_t parameter_count, std::size_t local_variables_count, String&& str):
		param_count(parameter_count), 
		locals_count(local_variables_count), 
		types(std::forward<String>(str)),
		hash_value(compute_hash())
	{
		
	}

	// std::basic_string to leverage SSO
	const std::size_t param_count;
	const std::size_t locals_count;
	const std::basic_string<char_type> types;
	const std::size_t hash_value;

private:
	std::size_t return_type_count() const
	{
		return types.size() - param_count;
	}
	std::size_t compute_hash() const
	{
		auto hasher = std::hash<std::string_view>{};
		// hash the return types
		auto hash_v = hasher(std::string_view(types.c_str(), return_type_count()));
		auto param_types_hash = hasher(std::string_view(types.c_str() + return_type_count(), param_count));
		// I'm refuse to add boost as a dependency just for hash_combine()
		hash_v ^= param_types_hash + 0x9e3779b9 + (hash_v << 6) + (hash_v >> 2);
		return hash_v;
	}
};

bool operator==(const wasm_function_signature& left, const wasm_function_signature& right)
{ 
	return left.hash_value == right.hash_value 
		and left.param_count == right.param_count
		and left.types == right.types;
}

bool operator==(const wasm_function_signature& left, const wasm_function_signature& right)
{
	return not (left == right);
} 

namespace std {

	template<>
	struct hash<wasm_function_signature>
	{
		std::size_t operator()(const wasm_function_signature& sig) const
		{ return sig.hash_value; }
	};

} /* namespace std */


using wasm_function_signature_handle = std::shared_ptr<wasm_function_signature>;



struct wasm_function
{
	using char_type = std::underlying_type_t<wasm_value_t>;
	wasm_function_signature_handle signature;
	const std::basic_string<char_type> code;
};




#endif /* WASM_FUNCTION_H */


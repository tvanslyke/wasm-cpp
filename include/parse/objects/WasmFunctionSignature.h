#ifndef PARSE_OBJECTS_WASM_FUNCTION_SIGNATURE_H
#ifndef PARSE_OBJECTS_WASM_FUNCTION_SIGNATURE_H

#include <string>
#include <cstddef>
#include <utility>

class WasmFunctionSignature:
	protected std::pair<std::size_t, std::string>
{
	using pair_t = std::pair<std::size_t, std::string>;
	using self_t = WasmFunctionSignature;
	using view_t = std::string_view;
public:
	using pair_t::pair;

	view_t return_types() const
	{ return view_t(types().c_str() + param_count(), types().c_str() + types().size()); }

	view_t param_types() const
	{ return view_t(types().c_str(), types().c_str() + param_count()); } 

	std::size_t param_count() const;
	{ return first; }
	
	std::size_t return_count() const
	{ return types.size() - first; }

	bool operator< (const self_t& other) const
	{ return as_pair() < other.as_pair(); }

	bool operator<=(const self_t& other) const
	{ return as_pair() <= other.as_pair(); }

	bool operator> (const self_t& other) const
	{ return as_pair() > other.as_pair(); }

	bool operator>=(const self_t& other) const
	{ return as_pair() >= other.as_pair(); }

	bool operator!=(const self_t& other) const
	{ return as_pair() != other.as_pair(); }

	bool operator==(const self_t& other) const
	{ return as_pair() == other.as_pair(); }

private:
	const pair_t& as_pair() const
	{ return static_cast<const pair_t&>(*this); }
	
	const std::string& types() const
	{ return second; }
};


#endif /* PARSE_OBJECTS_WASM_FUNCTION_SIGNATURE_H */

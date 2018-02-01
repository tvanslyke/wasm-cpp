#ifndef MODULE_PARSE_DEF_H
#define MODULE_PARSE_DEF_H

#include <tuple>
#include "parse/leb128/leb128.h"
#include "utilites/endianness.h"

template <class Integer>
struct parse_varint: public LEB128_Decoder<Integer>
{
	using value_type = Integer;
};
template <class Value>
struct parse_int
{
	using value_type = Value;
	template <class It>
	std::pair<value_type, It> operator()(It begin, It end) const
	{
		unsigned char bytes[sizeof(value_type)];
		for(std::size_t i = 0; i < sizeof(value_type); ++i)
			bytes[i] = *begin++;
		ValueType value;
		std::memcpy(&value, bytes, sizeof(value));
		value = le_to_system(value);
		return value;
	}
};

template <class Container>
struct parse_array_base
{
	using elem_type = typename Container::base;
	using value_type = Container;
	template <class It>
	std::pair<value_type, It> operator()(It begin, It end) const
	{
		ParseElem parse_elem;
		wasm_uint32_t len;
		std::tie(len, begin) = parse_varint<wasm_uint32_t>{}(begin, end);
		value_type elems;
		elems.reserve(len);
		for(elem_type elem; len > 0; --len)
		{
			std::tie(elem, begin) = parse_elem(begin, end);
			elems.push_back(std::move(elem));
		}
		return {elems, begin};
	}
};

template <class T>
using parse_array = parse_array_base<std::vector<T>>;
template <class T>
using parse_string_base = parse_array_base<std::basic_string<T>>;
using parse_string = parse_string_base<char>;

template <class ParseFunc>
struct parse_maybe
{
	using parsed_type = typename ParseFunc::value_type;
	using value_type = std::optional<T>;
	template <class It>
	std::pair<value_type, It> operator()(It begin, It end) const
	{
		value_type result;
		bool has_value;
		std::tie(has_value, begin) = leb128_decode_uint1(begin, end);
		if(has_value)
			std::tie(result, begin) = ParseFunc{}(begin, end);
		return {result, begin};			
	}
	
};

template <class ... ParseField>
struct parse_composite
{
	using value_type = std::tuple<typename ParseField::value_type ...>;
	template <class It>
	std::pair<value_type, It> operator()(It begin, It end) const
	{
		value_type v;
		tuple_elem_parse<0, It>{}(begin, end, v);
		return v;
	}
private:
	using parse_types = std::tuple<ParseField ...>;
	template <std::size_t I, class It>
	struct tuple_elem_parse
	{
		void operator()(It begin, It end, value_type& tup) const
		{
			using current_parser = std::tuple_element_t<I, parse_types>;
			std::tie(std::get<I>(tup), begin) = current_parser{}(begin, end);
			// if there are more tuple elements to parse, then parse 'em
			if constexpr(I + 1 < std::tuple_size_v<value_type>)
				tuple_elem_parse<I + 1, It>{}(begin, end, tup);
		}
	}
};

using parse_func_type = parse_composite<
				parse_varint<wasm_uint8_t>,
				parse_array<wasm_language_type>,
				parse_maybe<parse_int<wasm_language_type>>;


template <class It>
void extract_wasm_function_type(It begin, It end)
{
	
}








#endif /* MODULE_PARSE_DEF_H */

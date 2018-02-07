#ifndef PARSE_WASM_BINARY_PARSER_H
#define PARSE_WASM_BINARY_PARSER_H

#include "wasm_base.h"
#include "parse/leb128/leb128.h"
#include "function/wasm_function.h"


struct wasm_binary_parser
{
	template <class ConstIt>
	wasm_binary_parser(ConstIt begin_, ConstIt end_):
		begin(&(*begin_)), pos(begin), end(&(*end_))
	{
		
	}


	template <class Type>
	Type parse_direct()
	{
		assert_space_for(sizeof(Type));
		Type value;
		std::memcpy(&value, pos, sizeof(value));
		pos += sizeof(value);
		return value;
	}

	template <class Type>
	Type parse_direct_endian()
	{
		return le_to_system(parse_direct<Type>());
	}

	wasm_uint8_t parse_leb128_uint1();
	wasm_uint8_t parse_leb128_uint7();
	wasm_uint8_t parse_leb128_uint8();
	wasm_uint16_t parse_leb128_uint16();
	wasm_uint32_t parse_leb128_uint32();
	wasm_uint64_t parse_leb128_uint64();

	wasm_sint8_t  parse_leb128_sint7();
	wasm_sint8_t  parse_leb128_sint8();
	wasm_sint16_t parse_leb128_sint16();
	wasm_sint32_t parse_leb128_sint32();
	wasm_sint64_t parse_leb128_sint64();

	std::string parse_string()
	{ 
		std::string dest;
		auto count = get_count();
		dest.resize(count);
		read_sequence(dest.begin(), &wasm_binary_parser::parse_direct<char>);
		return dest;
	}

	std::size_t bytes_remaining() const;
	std::size_t bytes_consumed() const;
	std::size_t bytes_total() const;
private:
	inline wasm_uint32_t get_count()
	{ return parse_leb128_uint32(); }

	template <class DestIt, class T>
	std::size_t read_sequence(DestIt dest, T (wasm_binary_parser::* memfunc)())
	{
		auto count = get_count();
		for(auto n = count; n > 0; --n)
			*dest++ = ((*this).*memfunc)();
		return count;
	}

	void assert_space_for(std::size_t bytes) const;
	const char* begin;
	const char* pos;
	const char* end;
};



#endif /* PARSE_WASM_BINARY_PARSER_H */

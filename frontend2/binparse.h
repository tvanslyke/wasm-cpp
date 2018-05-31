#ifndef BINPARSE_H
#define BINPARSE_H
#include <iostream>
#include <cstdint>
#include <type_traits>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <string>
#include <utility>
#include <optional>
#include <stdexcept>
#include "leb128_u_parser.h"
#include "leb128_s_parser.h"
#include "../include/wasm_instruction.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/object/static_cast.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
namespace wasm::parse {
using namespace boost::spirit;

inline const auto i32_type_code = qi::copy(qi::byte_(0x7F));
inline const auto i64_type_code = qi::copy(qi::byte_(0x7E));
inline const auto f32_type_code = qi::copy(qi::byte_(0x7D));
inline const auto f64_type_code = qi::copy(qi::byte_(0x7C));
inline const auto anyfunc_type_code = qi::copy(qi::byte_(0x70));
inline const auto func_type_code = qi::copy(qi::byte_(0x60));
inline const auto empty_block_type_code = qi::copy(qi::byte_(0x40));

inline const auto value_type = qi::copy(
	qi::byte_[qi::_pass = (qi::_1 >= 0x7Du) and (qi::_1 <= 0x7Fu)]
);
inline const auto block_type = qi::copy(
	qi::byte_[qi::_pass = (qi::_1 == 0x40u) or ((qi::_1 >= 0x7Du) and (qi::_1 <= 0x7Fu))]
);
inline const auto language_type = qi::copy(
	qi::byte_[qi::_pass = ((qi::_1 >= 0x7Du) and (qi::_1 <= 0x7Fu)) 
		or (qi::_1 == 0x70u)
		or (qi::_1 == 0x60u)
		or (qi::_1 == 0x40u)
	]
);

inline const auto varuint1 =  qi::copy(
	qi::byte_[qi::_pass = qi::_1 < 2u]
);
inline const auto varuint1_bool = qi::copy(
	(qi::byte_(0b0000'0001u) >> qi::attr(true)) | (qi::byte_(0b0000'0000u) >> qi::attr(false))
);
inline const auto varuint7 =  qi::copy(
	qi::byte_[qi::_pass = qi::_1 <= 0b0111'1111u]
);
inline const auto varuint32 = qi::copy(
	leb128_u[qi::_pass = (qi::_1 <= std::numeric_limits<std::uint32_t>::max())]
);

	
inline const auto varint7 = qi::copy(
	qi::as<signed char>()[
		// match positive signed
		qi::byte_[qi::_pass = (qi::_1 <= 0b0011'1111u)]
		// match negative signed
		| qi::byte_[qi::_pass = (qi::_1 <= 0b0111'1111u) and (qi::_1 >= 0b0100'0000u)][qi::_1 |= 0b1000'0000u]
	]
);

inline const auto varint32 = qi::copy(
	leb128_s[
		_pass = (_1 <= std::numeric_limits<std::int32_t>::max())
			and (_1 >= std::numeric_limits<std::int32_t>::min())
	]
);
inline const auto varint64 = qi::copy(
	leb128_s[
		_pass = (_1 <= std::numeric_limits<std::int64_t>::max())
			and (_1 >= std::numeric_limits<std::int64_t>::min())
	]
);

template <class T>
auto varuint1_option(T&& value) 
{
	return qi::copy(qi::byte_(0) | (qi::byte_(1) >> std::forward<T>(value)));
};

template <class T, class U>
auto varuint1_choice(T&& first, U&& second)
{
	return qi::copy((qi::byte_(0) >> std::forward<T>(first)) | (qi::byte_(1) >> std::forward<U>(second)));
};


inline const auto global_type = qi::copy(
	value_type >> varuint1_bool
);

inline const auto elem_type = qi::copy(anyfunc_type_code);

inline const auto resizable_limits = qi::copy(varuint1_choice(varuint32 >> varuint32, varuint32));

inline const auto table_type = qi::copy(elem_type >> resizable_limits);

inline const auto memory_type = qi::copy(resizable_limits);

namespace external_kind {
	inline const auto Function = qi::copy(qi::byte_(0));
	inline const auto Table =    qi::copy(qi::byte_(1));
	inline const auto Memory =   qi::copy(qi::byte_(2));
	inline const auto Global =   qi::copy(qi::byte_(3));
}

enum class SectionType: unsigned char {
	Custom   = 0,
	Type     = 1,
	Import   = 2,
	Function = 3,
	Table    = 4,
	Memory   = 5,
	Global   = 6,
	Export   = 7,
	Start    = 8,
	Element  = 9,
	Code     = 10,
	Data     = 11
};

template <class It>
const qi::rule<It, std::pair<unsigned char, std::string>(), qi::locals<std::uint32_t>> module_section = qi::copy(
	(varuint7[qi::_pass = (qi::_1 <= 11u)]
	>> qi::omit[varuint32[qi::_a = qi::_1]]
	>> qi::repeat(qi::_a)[qi::char_])[([](const auto& v, auto&, auto&) {
		using std::get;
		std::cout << int(get<0>(v)) << " : ";
		std::cout << std::string(get<1>(v).begin(), get<1>(v).end()) << std::endl;
	})]
);

template <class It>
std::vector<std::pair<unsigned char, std::string>> extract_module_sections(It first, It last)
{
	std::vector<std::pair<unsigned char, std::string>> sections;
	const auto module_header = qi::copy(qi::lit(std::string("\0asm", 4)) >> qi::little_dword(0x01u));
	auto succeeded = qi::parse(
		first, 
		last, 
		module_header
	);
	if(not succeeded)
		throw std::runtime_error("Bad Module Header.");
	while(first != last)
	{
		succeeded = qi::parse(
			first, 
			last, 
			varuint32
		);
	}
	return sections;
}



using namespace wasm_opcode;
inline const auto valid_opcode = qi::copy(
	qi::byte_[qi::_pass = not boost::phoenix::ref(wasm_instruction_dne)(qi::_1)]
);

inline const auto block_type_opcode = qi::copy(
	qi::byte_[qi::_pass = (
		   qi::_1 == BLOCK
		or qi::_1 == LOOP 
		or qi::_1 == IF
		)
	]
);

inline const auto varuint32_opcode = qi::copy(
	qi::byte_[qi::_pass = (
		   qi::_1 == BR
		or qi::_1 == BR_IF
		or qi::_1 == IF
		or qi::_1 == CALL
		or qi::_1 == GET_LOCAL
		or qi::_1 == SET_LOCAL
		or qi::_1 == TEE_LOCAL
		or qi::_1 == GET_GLOBAL 
		or qi::_1 == SET_GLOBAL
		)
	]
);

inline const auto memory_opcode = qi::copy(
	qi::byte_[qi::_pass = (qi::_1 >= 0x28 and qi::_1 <= 0x38)]
);

inline const auto memory_grow_query_opcode = qi::copy(
	qi::byte_[qi::_pass = (qi::_1 >= 0x39 and qi::_1 <= 0x40)]
);

inline const auto br_table_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == BR_TABLE]);
inline const auto call_indirect_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == CALL_INDIRECT]);
inline const auto varint32_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == I32_CONST]);
inline const auto varint64_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == I64_CONST]);
inline const auto float32_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == F32_CONST]);
inline const auto float64_opcode = qi::copy(qi::byte_[qi::_pass = qi::_1 == F64_CONST]);


inline const auto memory_immediate = qi::copy(varuint32 >> varuint32);
template <class It>
inline const auto br_table_immediate = qi::copy(
	varuint32// varuint32_prefixed_sequence<It>(varuint32) > varuint32
);


template <class It>
inline const auto full_instruction = qi::copy(
	  (memory_opcode             > memory_immediate)
	| (memory_grow_query_opcode  > omit[varuint1])
	| (varuint32_opcode          > varuint32)
	| (block_type_opcode         > block_type)
	| (br_table_opcode           > br_table_immediate<It>)
	| (call_indirect_opcode      > (varuint32 > omit[varuint1]))
	| (varint32_opcode           > varint32)
	| (varint64_opcode           > varint64)
	| (float32_opcode            > qi::bin_float)
	| (float64_opcode            > qi::bin_double)
	| valid_opcode
);


auto exact_opcode(wasm_opcode::wasm_instruction instr)
{
	return qi::copy(byte_[qi::_pass = qi::_1 == instr]);
}

inline const auto initializer_expression = qi::copy(
	(
		  (exact_opcode(I32_CONST)  > varint32)
		| (exact_opcode(I64_CONST)  > varint64)
		| (exact_opcode(F32_CONST)  > qi::bin_float)
		| (exact_opcode(F64_CONST)  > qi::bin_double)
		| (exact_opcode(GET_GLOBAL) > varuint32)
	)
	> omit[exact_opcode(END)]
);



} /* namespace wasm::parse */
#endif /* BINPARSE_H */

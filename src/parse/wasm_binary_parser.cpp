#include "parse/wasm_binary_parser.h"



wasm_uint8_t  wasm_binary_parser::parse_leb128_uint1()
{ 
	auto results = leb128_decode_uint1(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_uint8_t  wasm_binary_parser::parse_leb128_uint7()
{ 
	auto results = leb128_decode_uint7(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_uint8_t  wasm_binary_parser::parse_leb128_uint8()
{ 
	auto results = leb128_decode_uint8(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_uint16_t wasm_binary_parser::parse_leb128_uint16()
{ 
	auto results = leb128_decode_uint16(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_uint32_t wasm_binary_parser::parse_leb128_uint32()
{ 
	auto results = leb128_decode_uint32(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_uint64_t wasm_binary_parser::parse_leb128_uint64()
{ 
	auto results = leb128_decode_uint64(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}

wasm_sint8_t  wasm_binary_parser::parse_leb128_sint7()
{ 
	auto results = leb128_decode_sint7(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_sint8_t  wasm_binary_parser::parse_leb128_sint8()
{ 
	auto results = leb128_decode_sint8(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_sint16_t wasm_binary_parser::parse_leb128_sint16()
{ 
	auto results = leb128_decode_sint16(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_sint32_t wasm_binary_parser::parse_leb128_sint32()
{ 
	auto results = leb128_decode_sint32(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}
wasm_sint64_t wasm_binary_parser::parse_leb128_sint64()
{ 
	auto results = leb128_decode_sint32(pos, end);
	pos = std::get<1>(results);
	return std::get<0>(results);
}



std::pair<wasm_value_t, bool> parse_initializer_expression()
{
	auto opcode = parser.parse_direct<opcode_t>();
	wasm_value_t value;
	bool initialized = false;
	switch(opcode)
	{
	case wasm_opcode::I32_CONST:
		value.s32 = parser.parse_leb128_sint32();
		assert(parser.parse_direct<opcode_t>() == opcode_t(wasm_opcode::END));
		initialized = true;
		break;
	case wasm_opcode::I64_CONST:
		value.s64 = parser.parse_leb128_sint64();
		assert(parser.parse_direct<opcode_t>() == opcode_t(wasm_opcode::END));
		initialized = true;
		break;
	case wasm_opcode::F32_CONST:
		value.f32 = parser.parse_direct<float>();
		assert(parser.parse_direct<opcode_t>() == opcode_t(wasm_opcode::END));
		initialized = true;
		break;
	case wasm_opcode::F64_CONST:
		value.f64 = parser.parse_direct<double>();
		assert(parser.parse_direct<opcode_t>() == opcode_t(wasm_opcode::END));
		initialized = true;
		break;
	case wasm_opcode::GET_GLOBAL:
		// set .u32 equal to the index of the global variable we depend on.
		value.u32 = parser.parse_leb128_uint32();
		assert(parser.parse_direct<opcode_t>() == opcode_t(wasm_opcode::END));
		break;
	default:
		assert(false and "Bad instruction in initializer expression.");
	}
	return {value, initialized};
}

















std::size_t wasm_binary_parser::bytes_remaining() const
{ return end - pos; }
std::size_t wasm_binary_parser::bytes_consumed() const
{ return pos - begin; }
std::size_t wasm_binary_parser::bytes_total() const
{ return end - begin; }

void wasm_binary_parser::assert_space_for(std::size_t bytes) const
{
	if(bytes > bytes_remaining())
		throw std::out_of_range("Bad length while parsing wasm binary.");
}











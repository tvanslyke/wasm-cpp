#include "parse/objects/WasmFunctionObject.cpp"
#include "parse/wasm_binary_parser.h"


const char* WasmFunctionObject::define_from_encoding(const char* begin, const char* end)
{
	if(is_defined())
		throw std::runtime_error("Attempt to define function twice");
	wasm_binary_parser parser(begin, end);
	parser.set_remaining(parser.parse_leb128_uint32());
	locals_count = 0;
	for(auto entry_count = parser.parse_leb128_uint32(); entry_count > 0; --entry_count)
	{
		locals_count += parser.parse_leb128_uint32();
		parser.parse_leb128_sint7();
	}
	code.resize(parser.bytes_remaining());
	parser.parse_raw_bytes(code.size(), code.begin());
	assert(parser.bytes_remaining() == 0);
	assert(code.size() > 0);
	assert(wasm_opcode::wasm_opcode_t(code.back()) == wasm_opcode::END);
	defined = true;
	return begin + parser.total_bytes();
}

bool WasmFunctionObject::is_defined() const
{
	return defined;
}


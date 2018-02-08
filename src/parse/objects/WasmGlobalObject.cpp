#include "parse/objects/WasmGlobalObject.h"
#include "wasm_instruction.h"


const char* WasmGlobalObject::define_from_encoding(const char* begin, const char* end)
{
	using opcode_t = wasm_opcode::opcode_t;
	wasm_binary_parse parser(begin, end);
	typcode = parser.parse_leb128_sint7();
	std::tie(value, initialized) = parser.parse_initializer_expression();
	defined = true;
	return begin + parser.bytes_consumed();
}


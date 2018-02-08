#include "parse/objects/WasmGlobalObject.h"
#include "wasm_instruction.h"
#include <algorithm>

const char* WasmTableObject::define_from_encoding(const char* begin, const char* end)
{
	wasm_binary_parse parser(begin, end);
	table = parser.parse_resizable_limits<wasm_uint32_t>();
	// TODO: use a true sentinal value in tables
	std::fill(table.begin(), table.end(), std::numeric_limits<wasm_uint32_t>::max());
	defined = true;
	return begin + parser.bytes_consumed();
}

const char* WasmTableObject::initialize_segment(std::size_t offset, std::size_t len, const char* data_begin, const char* data_end)
{
	if((offset > table.size()) or (table.size() - offset < len))
		throw std::out_of_range("Bad {offset, size} pair in table initialization.");
	defined_ranges.insert_range(offset, len);
	// assign the table elements
	wasm_binary_parser parser(data_begin, data_end);
	std::generate(table.begin() + offset, 
		table.begin() + offset + len, 
		std::bind(wasm_binary_parser::parse_leb128_uint32, parser));
	// return the pointer to the first not-yet-parsed byte
	auto endpos = data_begin + parser.bytes_consumed();
	assert(endpos <= data_end);
	return endpos;
}


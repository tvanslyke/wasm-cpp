#include "parse/objects/WasmGlobalObject.h"
#include "wasm_instruction.h"
#include <algorithm>

const char* WasmMemoryObject::define_from_encoding(const char* begin, const char* end)
{
	wasm_binary_parse parser(begin, end);
	memory = parser.parse_resizable_limits<wasm_byte_t>(page_size);
	// initialize to zero bytes (it already should be all zeros, but
	// the above code may change in the future for all we know)
	std::memset(memory.data(), 0, memory.size() * sizeof(*(memory.data())));
	defined = true;
	return begin + parser.bytes_consumed();
}

const char* WasmMemoryObject::initialize_segment(std::size_t offset, std::size_t len, const char* data_begin, const char* data_end)
{
	assert(data_begin < data_end);
	if((offset > memory.size()) or (memory.size() - offset < len))
		throw std::out_of_range("Bad {offset, size} pair in linear memory initialization.");
	else if(len > std::size_t(data_end - data_begin))
		throw std::out_of_range("Requested too large of a size in linear memory initialization.");
		
	defined_ranges.insert_range(offset, len);
	std::memcpy(memory.data() + offset, data_begin, len);
	return data_begin + len;
}



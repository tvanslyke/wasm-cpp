#ifndef PARSE_OBJECTS_WASM_MEMORY_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_MEMORY_OBJECT_H

#include "parse/objects/WasmObject.h"
#include "utilities/SegmentTracker.h"
#include <vector>

struct WasmMemoryObject: 
        public WasmObject
{
        inline WasmObjectKind get_type() const override 
        { return WasmObjectKind(WasmObjectKind::memory_typecode); }
	bool is_defined() const
	{ return defined; }

	const char* initialize_segment(std::size_t offset, std::size_t len, const char* begin, const char* end);
private:
	const char* define_from_encoding(const char* begin, const char* end) override;
	std::vector<wasm_byte_t> memory;
	// vector of pairs of indices which indicate which subranges have been defined 
	SegmentTracker defined_ranges;
	bool defined = false;
	static constexpr const std::size_t page_size = 1024 * 64;
};
	



#endif /* PARSE_OBJECTS_WASM_MEMORY_OBJECT_H */

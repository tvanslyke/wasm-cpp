#ifndef PARSE_OBJECTS_WASM_TABLE_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_TABLE_OBJECT_H

#include "parse/objects/WasmObject.h"
#include "utilities/SegmentTracker.h"
#include <vector>
struct WasmTableObject
{
	// construct from encoded type signature
	inline WasmObjectKind get_type() const override 
        { return WasmObjectKind(WasmObjectKind::function_typecode); }

	bool is_defined() const
	{ return defined; }
	
	const char* initialize_segment(std::size_t offset, std::size_t len, const char* begin, const char* end);
private:
	const char* define_from_encoding(const char* begin, const char* end) override;
	std::vector<wasm_uint32_t> table;
	SegmentTracker defined_ranges;
	bool defined = false;
};	


#endif /* PARSE_OBJECTS_WASM_TABLE_OBJECT_H */

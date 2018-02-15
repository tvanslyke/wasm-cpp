#ifndef PARSE_OBJECTS_WASM_TABLE_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_TABLE_OBJECT_H

#include "parse/objects/WasmObject.h"
#include "utilities/SegmentTracker.h"
#include <vector>
struct WasmTableObject
{
	// construct from encoded type signature
	inline WasmObjectKind get_kind() const override 
        { return WasmObjectKind(WasmObjectKind::function_typecode); }

	bool is_defined() const
	{ return defined; }
	
	const char* initialize_segment(std::size_t offset, std::size_t len, const char* begin, const char* end);
	static std::pair<WasmObjectType, const char*>
	make_type(const char* begin, const char* end, WasmObjectKind kind, const WasmModuleObject& module);
private:
	const char* define_from_encoding(const char* begin, const char* end) override;
	std::vector<wasm_uint32_t> table;
	wasm_uint32_t maximum;
	SegmentTracker defined_ranges;
	bool defined = false;
};	


#endif /* PARSE_OBJECTS_WASM_TABLE_OBJECT_H */

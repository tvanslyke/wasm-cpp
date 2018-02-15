#ifndef PARSE_OBJECTS_WASM_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_OBJECT_H

#include <string>
#include <unordered_map>
#include <cstdint>
#include "parse/objects/WasmObjectType.h"

struct WasmObject 
{
	virtual ~WasmObject() = default;

	WasmObjectKind get_kind() const
	{ return type.kind(); }
	
	virtual void initialize_segment(const char* begin, const char* end)
	{
		std::string message = "initialize_segment() invalid for object of type" + get_kind().name() + ".";
		throw std::runtime_error(message);
	}
	
	virtual bool is_defined() const = 0;

	const char* add_definition(const char* begin, const char* end)
	{
		return this->define_from_encoding(begin, end);
	}

protected:
	virtual const char* define_from_encoding(const char* begin, const char* end) = 0;
	WasmObjectType type;
};




#endif /* PARSE_OBJECTS_WASM_OBJECT_H */

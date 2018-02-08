#ifndef PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H

#include <string>
#include <unordered_map>
#include <cstdint>
#include <string_view>
#include "parse/objects/WasmObject.h"
#include "wasm_instruction.h"
#include "parse/objects/WasmFunctionSignature.h"

struct WasmFunctionObject: 
        public WasmObject
{
	// construct from encoded type signature
	template <class ... Args>
	WasmFunctionObject(Args&& ... args):
		signature(std::forward<Args>(args)...)
	{
		
	}
        inline WasmObjectType get_type() const override 
        { return WasmObjectType(WasmObjectType::function_typecode); }
	
	bool is_defined() const override
	{ return defined; }
	
	// property
	const WasmFunctionSignature signature;
private:
	const char* define_from_encoding(const char* begin, const char* end) override;
	std::basic_string<wasm_opcode::wasm_opcode_t> code;
	std::size_t locals_count = 0;
	bool defined = false;

};
	


#endif /* PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H */

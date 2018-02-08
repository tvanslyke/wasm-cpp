#ifndef PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H

#include "wasm_value.h"
#include "wasm_instruction.h"


struct WasmGlobalObject: 
        public WasmObject
{
	WasmGlobalObject
        inline WasmObjectType get_type() const override 
        { return WasmObjectType(WasmObjectType::global_typecode); }
	
	bool is_defined() override
	{ return defined; }

	bool is_initialized() const
	{ return is_initialized; }

	wasm_uint32_t dependancy_index() const
	{
		assert(is_defined());
		assert(not is_initialized());
		return value.u32;
	}
private:
	const char* define_from_encoding(const char* begin, const char* end) override;
	wasm_sint8_t typecode;
	wasm_value_t value;
	// TODO: something more compact than three bools...
	bool is_mutable;
	bool defined = false;
	bool initialized = false;
};
	


#endif /* PARSE_OBJECTS_WASM_FUNCTION_OBJECT_H */

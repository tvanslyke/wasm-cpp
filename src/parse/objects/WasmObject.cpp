#include "parse/objects/WasmObject.h"

WasmTypeObject::WasmTypeObject(wasm_byte_t typecode):
	typecode_(typecode)
{
	if(value() >= 4)
		throw std::runtime_error("Bad value_type code (" 
			+ std::to_string(value()) 
			+ ") encountered when constructing WasmObjectKind.");
}

const std::string& WasmTypeObject::name() const
{ return type_names_[typecode_]; }

wasm_byte_t WasmTypeObject::value() const
{ return typecode_; }

const std::array<std::string, 4> WasmTypeObject::type_names_{
	"Function",
	"Table",
	"Linear Memory",
	"Global Variable"
};




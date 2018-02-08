#ifndef PARSE_OBJECTS_WASM_OBJECT_H
#ifndef PARSE_OBJECTS_WASM_OBJECT_H

#include <string>
#include <unordered_map>
#include <cstdint>

struct WasmObjectType {

	WasmObjectType(wasm_byte_t typecode);
	const std::string& name() const;
	wasm_byte_t value() const;

	static const wasm_byte_t function_typecode = 0;
	static const wasm_byte_t table_typecode    = 1;
	static const wasm_byte_t memory_typecode   = 2;
	static const wasm_byte_t global_typecode   = 3;
private:
	const wasm_byte_t typecode_;
	static const std::array<std::string, 4> type_names_;
};

struct WasmObject 
{
	virtual ~WasmObject() = default;
	virtual WasmObjectType get_type() const = 0;
	
	virtual void initialize_segment(const char* begin, const char* end)
	{
		std::string message = "initialize_segment() invalid for object of type" + get_type().name() + ".";
		throw std::runtime_error(message);
	}
	
	virtual bool is_defined() const = 0;

	const char* add_definition(const char* begin, const char* end)
	{
		return this->define_from_encoding(begin, end);
	}

protected:
	virtual const char* define_from_encoding(const char* begin, const char* end) = 0;
};

	


#endif /* PARSE_OBJECTS_WASM_OBJECT_H */

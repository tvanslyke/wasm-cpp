#ifndef PARSE_OBJECTS_WASM_OBJECT_TYPE_H
#ifndef PARSE_OBJECTS_WASM_OBJECT_TYPE_H

#include <string>
#include <unordered_map>
#include <cstdint>

struct WasmModuleObject;

struct WasmObjectKind {

	WasmObjectKind(wasm_byte_t typecode);
	const std::string& name() const;
	wasm_byte_t value() const;

	static const wasm_byte_t function_typecode = 0;
	static const wasm_byte_t table_typecode    = 1;
	static const wasm_byte_t memory_typecode   = 2;
	static const wasm_byte_t global_typecode   = 3;

	bool operator==(const WasmObjectKind& other)
	{ return typecode_ == other.typecode_; }
	
	bool operator!=(const WasmObjectKind& other)
	{ return not ((*this) == other); }
private:
	const wasm_byte_t typecode_;
	static const std::array<std::string, 4> type_names_;
};

struct WasmObjectType:
	public WasmObjectKind
{
	WasmObjectKind(std::unqiqe_ptr<WasmObjectTypeImpl>&& impl):
		type_impl(std::move(impl))
	{
		
	}

	WasmObjectKind kind() const
	{ return static_cast<const WasmObjectKind&>(*this) };
	
	~WasmObjectKind() = default;
	
	bool operator==(const WasmObjectType& other)
	{ 
		return (kind() == other.kind()) and (type_impl->equals(*(other.type_impl)));
	}
	
	bool operator!=(const WasmObjectKind& other)
	{ return not ((*this) == other); }
	
	struct WasmObjectTypeImpl {
		WasmObjectTypeImpl(const WasmModuleObject* mod):
			module(mod)
		{
			
		}
		virtual ~WasmObjectTypeImpl();
		virtual bool equals(const WasmObjectTypeImpl& other) const
	protected:
		const WasmModuleObject* module;
	};

	static std::pair<WasmObjectType, const char*>
	make(const char* begin, const char* end, WasmObjectKind kind, const WasmModuleObject& module);
	
private:
	WasmObjectKind() = default;
	std::unique_ptr<WasmObjectTypeImpl> type_impl;
};




#endif /* PARSE_OBJECTS_WASM_OBJECT_TYPE_H */

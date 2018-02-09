#include "parse/objects/WasmProgramObjects.h"



void WasmProgramObjects::add_exports()
{
	for(const auto& def: module_defs)
	{
		const auto& data = def.export_section();
		auto parser = wasm_binary_parser(data.begin(), data.end());
		auto& module = get_module(def.get_name());
		for(wasm_uint32_t count = parser.parse_leb128_uint32(); count > 0; --count)
		{
			auto field_name = parser.parse_string();
			auto kind = parser.parse_direct<wasm_uint8_t>();
			auto exported_object = add_empty_object(WasmTypeObject(kind));
			auto index = parser.parse_leb128_uint32();
			module.add_export(std::move(field_name), exported_object, index);
		}
	}
}

void WasmProgramObjects::add_imports()
{
	for(const auto& def: module_defs)
	{
		const auto& data = def.export_section();
		auto parser = wasm_binary_parser(data.begin(), data.end());
		auto& module = get_module(def.get_name());
		for(wasm_uint32_t count = parser.parse_leb128_uint32(); count > 0; --count)
		{
			auto module_name = parser.parse_string();
			auto field_name = parser.parse_string();
			auto kind = parser.parse_direct<wasm_uint8_t>();
			auto type = WasmObjectKind(kind);
			auto imported_object = get_module(module_name).get_export(field_name);
			assert(imported_object.get_type() == type);
			module.add_import(std::move(field_name), imported_object);
		}
	}
}

void WasmProgramObjects::add_signatures()
{
	for(const auto& def: module_defs)
	{
		const auto& data = def.type_section();
		auto parser = wasm_binary_parser(data.begin(), data.end());
		auto& module = get_module(def.get_name());
		wasm_uint32_t count = parser.parse_leb128_uint32();
		module.reserve_signatures(count);
		for(; count > 0; --count)
		{
			auto sig = std::make_unique<WasmFunctionSignature>(std::move(parser.parse_function_signature()));
			auto [sig, _] = signatures.insert(std::move(sig));
			module.add_signature(sig->get());
		}
	}
}

void WasmProgramObjects::add_functions()
{
	for(const auto& def: module_defs)
	{
		const auto& decl_data = def.function_section();
		const auto& body_data = def.code_section();
		auto decl_parser = wasm_binary_parser(decl_data.begin(), decl_data.end());
		auto body_parser = wasm_binary_parser(body_data.begin(), body_data.end());
		auto& module = get_module(def.get_name());
		wasm_uint32_t count = decl_parser.parse_leb128_uint32();
		{
			auto function_body_count = body_parse.parse_leb128_uint32(); 
			assert((count == code_body_count) && "number of function declarations and definitions must match!");
		}
		module.reserve_functions(count);
		for(wasm_uint32_t i = 0; i < count; ++i)
		{
			auto sig_pos = decl_parser.parse_leb128_uint32();
			auto sig = module.get_signature(sig_pos);
			auto body_bytes = 
			
			
		}
	}
}







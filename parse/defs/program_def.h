#ifndef PARSE_DEFS_FUNCTION_DEFS_H
#define PARSE_DEFS_FUNCTION_DEFS_H
#include "wasm_base.h"
#include "function/wasm_function.h"
#include "parse/wasm_binary_parser.h"


struct wasm_program_def
{
	void add_module(const wasm_module_def& module);
private:
	
	
	using initializer_expression_t = std::variant<std::size_t, wasm_value_t>;
	
	struct function_def_t
	{
		wasm_code_string_t code;
		std::size_t param_count;
		std::size_t return_count;
		std::size_t locals_count;
		func_sig_t sig;
	};

	struct global_variable_def_t
	{
		wasm_language_type type;
		std::optional<wasm_value_t> initial_value;
		initializer_expression_t initializer;
		bool mutability;
	};

	struct linear_memory_def_t
	{
		struct init_segment_t {
			initializer_expression_t offset_initializer;
			std::vector<wasm_byte_t> data;
		}
		std::size_t initial_size;
		std::optional<std::size_t> maximum;
		std::vector<init_segment_t> init_segments;
		std::vector<wasm_byte_t> initial_memory;
	};

	struct table_def_t
	{
		struct init_segment_t {
			initializer_expression_t offset_initializer;
			std::vector<std::size_t> function_indices;
		}
		std::size_t initial_size;
		std::optional<std::size_t> maximum;
		wasm_language_type element_type;
		std::vector<init_segment_t> init_segments;
		std::vector<std::size_t> functions;
	};
	
	enum external_kind {
		function = 0,
		table = 1,
		memory = 2,
		global = 3
	};

	struct import_t 
	{
		import_t(external_kind kind_, std::size_t index_):
			kind(kind), index(index), defined(false)
		{
			
		}
		external_kind kind;
		std::size_t index;
		bool defined = false;
	};
	
	struct export_t: public import_t
	{
		export_t(external_kind kind_, std::size_t index_):
			import_t(kind_, index_)
		{
			defined = true;
		}
	};

	struct module_mappings_t
	{
		using mapping_t = std::vector<std::ptrdiff_t>;
		std::vector<func_sig_id_t> function_signatures;
		std::ptrdiff_t start_function = -1;
		mapping_t functions;
		mapping_t globals;
		mapping_t memories;
		mapping_t tables;

		void map_index(external_kind kind, std::ptrdiff_t to_index, std::ptrdiff_t from_index = -1)
		{
			auto map_func = [](auto& mapping, auto to, auto from) {
				if(from_index < 0)
				{
					mapping.push_back(to);
					return;
				}
				else if(std::ptrdiff_t(mapping.size()) < from)
				{
					mapping.resize(std::size_t(from) + 1, -1);
				}
				mapping[from] = to;
			};
			switch(kind)
			{
			case external_kind::function:
				map(functions, to_index, from_index);
				break;
			case external_kind::table:
				map(tables, to_index, from_index);
				break;
			case external_kind::memory:
				map(memories, to_index, from_index);
				break;
			case external_kind::global:
				map(globals, to_index, from_index);
				break;
			default:
				throw std::runtime_error("Bad 'external_kind' encountered in module_mapping_t::map_index().");
			}
		}
	};



	void parse_module(const wasm_module_def& module)
	{
		parse_function_signatures(module);
		
	}

	/* 
	 * Register all of the function signatures defined in the type section
	 * with the function signature registrar and fill mappings.function_signatures
	 * with each of the corresponding ids returned from the registrar.
	 */
	void parse_function_signatures(const wasm_module_def& module, module_mappings_t& mappings)
	{
		using typecode_t = std::underlying_type_t<wasm_language_type>;
		const auto& data = module.type_section();
		auto begin = data.begin();
		auto end = data.end();
		wasm_binary_parser parser(begin, end);
		
		auto count = parser.parse_leb128_uint32();
		mappings.function_signatures.reserve(count);
		for(std::size_t i = 0; i < count; ++i)
		{
			auto sig_id = parse_function_type(parser);
			mappings.function_signatures.push_back(sig_id);
		}
	}

	func_sig_id_t parse_function_type(wasm_binary_parser& parser)
	{
		using typecode_t = std::underlying_type_t<wasm_language_type>;
		auto validate_typecode = [](typecode_t tc) {
			return tc == wasm_language_type::i32
				or tc == wasm_language_type::i64
				or tc == wasm_language_type::f32
				or tc == wasm_language_type::f64;
		};
		auto assign_next_typecode = [&](typecode_t& tc) {
			tc = parser.parse_direct<typecode_t>();
			if(not validate_typecode(typecode))
				throw std::runtime_error("Bad 'value_type' encountered while "
					"parsing 'value_type's in function signature.");
		};
		std::basic_string<typecode_t> sig_string;
		wasm_uint32_t param_count;
		if(parser.parse_leb128_uint7() != wasm_language_type::func)
			throw std::runtime_error("Bad 'form' field encountered while "
				"parsing function signature.");
		
		auto param_count = parser.parse_leb128_uint32()
		sig_string.resize(param_count, 0);
		// get the parameter types
		std::for_each(sig_string.begin(), sig_string.end(), assign_next_typecode);
		sig_string.resize(sig_string.size() + parser.parse_leb128_uint32());
		// get the return types
		std::for_each(sig_string.begin() + param_count, sig_string.end(), assign_next_typecode);
		return sig_registrar.get_signature(sig_string, param_count);
	}


	void parse_tables(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.table_section();
		wasm_binary_parser parser(data.begin(), data.end());
		
		auto parse_table_entry = [&]()
		{
			table_def_t table_def;
			table_def.element_type = parser.parse_leb128_uint7();
			bool has_max_size = parser.parse_leb128_uint1();
			table_def.initial_size = parser.parse_leb128_uint32();
			if(has_max_size)
				table_def.maximum = parser.parse_leb128_uint32();
			
			return table_def;
		};
		auto count = parser.parse_leb128_uint32();
		assert(mappings.tables.size() <= count);
		mappings.tables.resize(count, -1);
		
		table_def_t table_def;
		for(wasm_uint32_t i = 0; i < count; ++i)
		{
			table_def = parse_table_entry();
			auto index = add_table_definition(std::move(table_def), mappings.tables[i]);
			if(mappings.tables[i] < 0)
				mappings.tables[i] = index;
			else
				assert(mappings.tables[i] == index);
		}
	}

	void parse_elements(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.element_section();
		wasm_binary_parser parser(data.begin(), data.end());
		auto& tables = mappings.tables;
		auto parse_segment = [&]()
		{
			std::size_t table_index = parser.parse_leb128_uint32();
			assert(table_index < tables.size());
			assert(tables[table_index] >= 0);
			table_index = static_cast<std::size_t>(tables[table_index]);
			assert(table_defs.size() < table_index);
			auto& def = table_defs[table_index];
			auto init_expr = parse_initializer_expression(parser);
			std::vector<std::size_t> func_indices;
			func_indices.resize(parser.parse_leb128_uint32());
			for(auto& ind: func_indices)
			{
				auto local_index = parser.parse_leb128_uint32();
				assert(mappings.functions.size() > local_index);
				assert(mappings.functions[local_index] > -1);
				ind = mappings.functions[local_index];
			}
			def.init_segments.emplace_back(std::move(init_expr), std::move(func_indices));
		};
		auto count = parser.parse_leb128_uint32();
		for(; count > 0; --count)
			parse_segment();
	}

	void parse_memories(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.memory_section();
		wasm_binary_parser parser(data.begin(), data.end());
		
		auto parse_memory_entry = [&]()
		{
			linear_memory_def_t def;
			bool has_max_size = parser.parse_leb128_uint1();
			def.initial_size = parser.parse_leb128_uint32();
			if(has_max_size)
				def.maximum = parser.parse_leb128_uint32();
			return def;
		};
		auto count = parser.parse_leb128_uint32();
		assert(mappings.memories.size() <= count);
		mappings.memories.resize(count, -1);
		
		linear_memory_def_t memory_def;
		for(wasm_uint32_t i = 0; i < count; ++i)
		{
			memory_def = parse_memory_entry();
			auto index = add_linear_memory_definition(std::move(memory_def), mappings.memories[i]);
			if(mappings.memories[i] < 0)
				mappings.memories[i] = index;
			else
				assert(mappings.memories[i] == index);
		}
	}

	void parse_data(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.data_section();
		wasm_binary_parser parser(data.begin(), data.end());
		
		auto& memories = mappings.memories;

		auto parse_segment = [&]()
		{
			std::size_t memory_index = parser.parse_leb128_uint32();
			assert(memory_index < memories.size());
			assert(memories[memory_index] >= 0);
			memory_index = static_cast<std::size_t>(memories[memory_index]);
			assert(linear_memory_defs.size() < memory_index);
			auto& def = linear_memory_defs[memory_index];
			auto init_expr = parse_initializer_expression(parser);
			std::vector<wasm_byte_t> init_data;
			init_data.resize(parser.parse_leb128_uint32());
			for(auto& byte: init_data)
				byte = parser.parse_direct<wasm_byte_t>();
			def.init_segments.emplace_back(std::move(init_expr), std::move(init_data));
		};
		auto count = parser.parse_leb128_uint32();
		for(; count > 0; --count)
			parse_segment();
	}

	void parse_globals(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.global_section();
		wasm_binary_parser parser(data.begin(), data.end());
		
		auto parse_global = [&]()
		{
			global_variable_def_t def;
			def.type = parser.parse_leb128_sint7();
			def.mutability = parse.parse_leb128_uint1();
			def.initializer = parse_initializer_expression(parser);
			return def;
		};
		auto count = parser.parse_leb128_uint32();
		assert(mappings.globals.size() <= count);
		mappings.globals.resize(count, -1);
		
		global_variable_def_t global_def;
		for(wasm_uint32_t i = 0; i < count; ++i)
		{
			global_def = parse_global();
			auto index = add_global_variable_definition(std::move(global_def), mappings.globals[i]);
			if(mappings.globals[i] < 0)
				mappings.globals[i] = index;
			else
				assert(mappings.globals[i] == index);
		}
	}

	void parse_imports(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.import_section();
		auto begin = data.begin();
		auto end = data.end();
		wasm_binary_parser parser(begin, end);

		auto count = parser.parse_leb128_uint32();
		std::string import_name;
		for(auto n = count; n > 0; --n)
		{
			// get the module name
			import_name = parser.parse_string();
			// get the field name
			import_name += parser.parse_string();
			auto kind = parser.parse_direct<wasm_uint8_t>();
			// get the program-wide index of the import definition
			auto index = resolve_import(import_name, kind);
			mappings.map_index(kind, index);
		}
	}
	
	template <class DestIt>
	void parse_exports(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.export_section();
		auto begin = data.begin();
		auto end = data.end();
		wasm_binary_parser parser(begin, end);

		std::string export_name = module.get_name();
		std::size_t name_base_size = export_name.size();

		auto count = parser.parse_leb128_uint32();
		for(auto n = count; n > 0; --n)
		{
			export_name.resize(name_base_size);
			// get the field name
			export_name += parser.parse_string();
			auto kind = parser.parse_direct<wasm_uint8_t>();
			// module-local index
			auto local_index = parser.parse_leb128_uin32();
			// get or the program-wide index of the import definition
			auto index = add_export(import_name, kind);
			mappings.map_index(kind, index, local_index);
		}
	}
	
	void parse_functions(const wasm_module_def& module, module_mappings_t& mappings)
	{
		// construct parsers for the function declarations and definitions, respectively
		const auto& sig_data_string = module.function_section();
		const auto& code_data_string = module.code_section();
		wasm_binary_parser sig_parser(sig_data_string.begin(), sig_data_string.end());
		wasm_binary_parser code_parser(code_data_string.begin(), code_data_string.end());
		// find out how many functions are defined in the module.  
		auto count = sig_parser.parse_leb128_uint32();
		// make sure both the declaration and definition sections agree on how many functions
		// are defined in the module
		if(auto body_count = code_parser.parse_leb128_uint32();
			body_count != count)
		{
			// if they disagree, throw an error
			std::string message = "Mismatched number of function declarations (";
			message += std::to_string(count);
			message += ") and function definitions (";
			message += std::to_string(body_count);
			message += ").";
			throw std::runtime_error(message);
		}
		// also make sure that the import and export sections agreed with this as well.
		// the imports and exports are parsed first, so may will have added indices to the
		// mappings.functions vectors.
		// in the future, this should be made more than an assert
		assert(mappings.functions.size() <= count);
		// allocate enough memory in the functions vector for the definitions we're about to add
		// a function whose index is -1 has not yet been given a spot in the program-wide function 
		// vector.  a function who does have an index that is greater than -1 was exported by this 
		// module and imported by another.  the program-wide index for that function was decided
		// while parsing the imports of another module.  when the exports for this module were 
		// parsed, we assigned that already-decided, program-wide index to the correct spot.
		// this ensures that everyone agrees on where imported/exported function definitions live.
		mappings.functions.resize(count, -1);
		wasm_function_def_t func_def;
		for(wasm_uint32_t i = 0; i < count; ++i)
		{
			func_def = parse_function_body(code_parser);
			func_def.signature = mappings.function_signatures[sig_parser.parse_leb128_uint32()];
			func_def.return_count = sig_registrar.get_return_count_for(func_def.signature);
			func_def.param_count = sig_registrar.get_parameter_count_for(func_def.signature);
			auto index = add_function_definition(std::move(func_def), mappings.functions[i]);
			if(mappings.functions[i] < 0)
				mappings.functions[i] = index;
			else
				assert(mappings.functions[i] == index);
		}
	}

	void add_start_function(const wasm_module_def& module, module_mappings_t& mappings)
	{
		const auto& data = module.export_section();
		if(data.size() == 0)
			throw std::runtime_error("Attempt to use start function from "
						"module which has no start function.");
		else if(program_start_function > 0)
			throw std::runtime_error("Attempt to redefine program start function.";
		assert(not program_main_module_added);
		auto begin = data.begin();
		auto end = data.end();
		wasm_binary_parser parser(data.begin(), data.end());
		auto local_index = parser.parse_leb128_uint32();
		assert(mappings.functions.size() < local_index);
		auto index = mappings.functions[local_index];
		assert(index > -1);
		assert(function_defs.size() > index);
		program_start_function = index;
	}

	wasm_function_def_t parse_function_body(wasm_binary_parser& parser)
	{
		wasm_function_def_t func_def;
		std::ptrdiff_t bytes_in_body = parser.parse_leb128_uint32();
		std::ptrdiff_t initial_offset = parser.bytes_consumed();
		auto bytes_consumed = [&](){ 
			return parser.bytes_consumed() - initial_offset;
		};
		auto local_entry_count = parser.parse_leb128_uint32();
		func_def.locals_count = 0;
		assert(bytes_consumed() < bytes_in_body);
		for(std::size_t i = 0; i < local_entry_count; ++i)
		{
			func_def.locals_count += parser.parse_leb128_uint32();
			// TODO: use this type info for a debug mode or something
			assert(bytes_consumed() < bytes_in_body);
			[[maybe_unused]] 
			auto locals_types = parser.parse_direct<wasm_language_type>();
			assert(bytes_consumed() < bytes_in_body);
		}
		func_def.code.resize(bytes_in_body - bytes_consumed());
		for(auto& opcode: func_def.code)
			opcode = parser.parse_direct<wasm_opcode_t>();
		return func_def;
	}

	initializer_expression_t 
	parse_initializer_expression(wasm_binary_parser& parser, const module_mappings_t& mappings) const
	{
		initializer_expression_t init_expr;
		assert(parser.bytes_remaining() > 1);
		auto opcode = parse_opcode(parser);
		switch(opcode)
		{
		case wasm_opcode::I32_CONST:
			init_expr = wasm_value_t{parser.parse_leb128_sint32()};
			break;
		case wasm_opcode::I64_CONST:
			init_expr = wasm_value_t{parser.parse_leb128_sint64()};
			break;
		case wasm_opcode::F32_CONST:
			init_expr = wasm_value_t{parser.parse_direct<float>()};
			break;
		case wasm_opcode::F64_CONST:
			init_expr = wasm_value_t{parser.parse_direct<double>()};
			break;
		case wasm_opcode::GET_GLOBAL:
			auto idx = parser.parse_leb128_uint32();
			assert(idx > mappings.globals.size());
			if(mappings.globals[idx] < 0)
				throw std::runtime_error("Initializer expressions involving "
							"non-imported globals are not permitted");
			// replace module-local global variable index with program-wide index
			init_expr = std::size_t(mapping.globals[idx]);
			break;
		default:
			throw std::runtime_error("Only GET_GLOBAL and <type>.CONST operations "
						"are supported in initializer expressions. (found " 
						+ std::to_string(opcode) ")");
		}
		return init_expr;
	}

	void finalize_code(wasm_code_string_t& code, module_mapping_t& mappings);

	static inline wasm_opcode::wasm_opcode_t parse_opcode(wasm_binary_parser& parser)
	{
		return parser.parse_direct<wasm_opcode::wasm_opcode_t>();
	}
	/*
	 * Walk through all of the function definitions in this module and 
	 * replace any variable-length immediates with their respective
	 * fixed-length values.  Additionally replace any occurences of 
	 * module-local function/memory/global/table indices with their respective
	 * program-wide indices. 
	 */
	void finalize_function_definitions(module_mappings_t& mappings);


	std::size_t resolve_import(const std::string& name, external_kind kind)
	{
		auto [import_pos, insertion_happened] = export_map.try_emplace(name, kind, 0);
		// check that the import/export types match
		if(insertion_happened)
		{
			import_pos->index = add_empty_definition(kind);
		}
		else if(kind != import_pos->kind)
		{
			if(import_pos->defined)
				// export declared a different type than the import
				throw std::runtime_error("Validation error: import/export types don't match for " + name);
			else
				// another import declared a different type for this name than we expected
				throw std::runtime_error("Validation error: import type disagreement for " + name);
		}
		return import_pos->index;
	}

	std::size_t add_export(const std::string& name, external_kind kind)
	{
		auto [import_pos, insertion_happened] = export_map.try_emplace(name, kind, 0);
		// check that the import/export types match
		if(insertion_happened)
		{
			import_pos->index = add_empty_definition(kind);
		}
		else if(not insertion_happened)
		{
			if(import_pos->defined)
				throw std::runtime_error("Double definition of export " + name);
			else if(import_pos->kind != exprt.kind)
				// export declared a different type than the import
				throw std::runtime_error("Validation error: import/export types don't match for " + name);
			else
				import_pos->defined = true;
		}
		return import_pos->index;
	}
	
	std::size_t add_empty_definition(external_kind kind)
	{
		switch(kind)
		{
		case external_kind::function:
			index = function_defs.emplace_back();
			return function_defs.size();
			break;
		case external_kind::table:
			index = table_defs.emplace_back();
			return table_defs.size();
			break;
		case external_kind::memory:
			index = linear_memory_defs.emplace_back();
			return linear_memory_defs.size();
			break;
		case external_kind::global:
			global_variable_defs.emplace_back();
			return global_variable_defs.size();
			break;
		default:
			throw std::runtime_error("Bad 'external_kind' encountered when resolving import " + name);
		}
	}
	
	template <class Def>
	std::size_t add_function_definition(Def&& def, std::ptrdiff_t idx = -1)
	{
		static const std::string def_name = "function";
		return add_program_definition(
			std::forward<Def>(def), 
			&wasm_program_def::function_defs, 
			def_name,
			idx
		);
	}

	template <class Def>
	std::size_t add_table_definition(Def&& def, std::ptrdiff_t idx = -1)
	{
		static const std::string def_name = "table";
		return add_program_definition(
			std::forward<Def>(def), 
			&wasm_program_def::table_defs, 
			def_name,
			idx
		);
	}

	template <class Def>
	std::size_t add_linear_memory_definition(Def&& def, std::ptrdiff_t idx = -1)
	{
		static const std::string def_name = "linear memory";
		return add_program_definition(
			std::forward<Def>(def), 
			&wasm_program_def::linear_memory_defs, 
			def_name,
			idx
		);
	}
	
	template <class Def>
	std::size_t add_global_variable_definition(Def&& def, std::ptrdiff_t idx = -1)
	{
		static const std::string def_name = "global variable";
		return add_program_definition(
			std::forward<Def>(def), 
			&wasm_program_def::global_variable_defs, 
			def_name,
			idx
		);
	}


	template <class Definition, class MemberType>
	std::size_t add_program_definition(Definition&& def, 
					MemberType wasm_program_def::* member, 
					const std::string& def_type_name, 
					std::ptrdiff_t idx = -1)
	{
		auto& defs = this->*member;
		if(idx < 0)
			defs.emplace_back(std::forward<Definition>(def));
		else if(static_cast<std::size_t>(idx) > function_defs.size())
			throw std::out_of_range("Attempt to define " + def_type_name + " at out-of-bounds index.");
		else if(function_defs[idx].has_value())
			throw std::out_of_range("Attempt to define an already-defined " + def_type_name + ".");
		else 
			defs[idx] = std::forward<Definition>(def);
		return idx < 0 ? defs.size() - 1 : idx;
	}
	
	void resolve_initializer_expressions()
	{
		
	}

	std::variant<std::size_t, wasm_value_t> eval_initializer(const initializer_expression_t& init_expr)
	{
		std::variant<std::size_t, wasm_value_t> result;
		assert(init_expr.size() > 0);
		wasm_binary_parser parser(init_expr.begin(), init_expr.end());
		auto opcode = parse_opcode(parser);
		switch(opcode)
		{
		case wasm_opcode::I32_CONST:
			result = wasm_value_t{parser.parse_leb128_sint32()};
			break;
		case wasm_opcode::I64_CONST:
			result = wasm_value_t{parser.parse_leb128_sint32()};
			break;
		case wasm_opcode::F32_CONST:
			result = wasm_value_t{parser.parse_leb128_sint32()};
			break;
		case wasm_opcode::F64_CONST:
			result = wasm_value_t{parser.parse_leb128_sint32()};
			break;
		case wasm_opcode::GET_GLOBAL:
			std::size_t 
			break;
		default:
			throw std::runtime_error("Unsupported opcode " + std::to_string(opcode) + " in initializer expression.");
		}
		assert(parser.bytes_remaining() == 1);
		assert(parse_opcode(parser) == wasm_opcode::END);
	}
	
	std::unordered_map<std::string, export_t> export_map;
	FunctionSignatureRegistrar sig_registrar;
	std::vector<std::optional<function_def_t>> function_defs;
	std::vector<std::optional<global_variable_def_t>> global_variable_defs;
	std::vector<std::optional<linear_memory_def_t>> linear_memory_defs;
	std::vector<std::optional<table_def_t>> table_defs;
	std::ptrdiff_t program_start_function = -1;
	bool program_main_module_added = false;
};

#endif /* PARSE_DEFS_FUNCTION_DEFS_H */

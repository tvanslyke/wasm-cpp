#ifndef PARSE_DEFS_FUNCTION_DEFS_H
#define PARSE_DEFS_FUNCTION_DEFS_H
#include "wasm_base.h"
#include "function/wasm_function.h"
#include "parse/wasm_binary_parser.h"
#include "parse/wasm_module_def.h"
#include "wasm_instruction_types.h"

struct wasm_program_def
{
	void add_module(const wasm_module_def& module, bool is_main);
private:
	
	using initializer_expression_t = std::variant<std::size_t, wasm_value_t>;
	
	struct function_def_t
	{
		wasm_code_string_t code;
		std::size_t param_count;
		std::size_t return_count;
		std::size_t locals_count;
		func_sig_id_t sig;
		bool finalized = false;
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
		};
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
		};
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

		void map_index(external_kind kind, std::ptrdiff_t to_index, std::ptrdiff_t from_index = -1);
	};


	void parse_module(const wasm_module_def& module);

	/* 
	 * Register all of the function signatures defined in the type section
	 * with the function signature registrar and fill mappings.function_signatures
	 * with each of the corresponding ids returned from the registrar.
	 */
	void parse_function_signatures(const wasm_module_def& module, module_mappings_t& mappings);

	func_sig_id_t parse_function_type(wasm_binary_parser& parser);

	void parse_tables(const wasm_module_def& module, module_mappings_t& mappings);

	void parse_elements(const wasm_module_def& module, module_mappings_t& mappings);

	void parse_memories(const wasm_module_def& module, module_mappings_t& mappings);

	void parse_data(const wasm_module_def& module, module_mappings_t& mappings);

	void parse_globals(const wasm_module_def& module, module_mappings_t& mappings);

	void parse_imports(const wasm_module_def& module, module_mappings_t& mappings);
	
	void parse_exports(const wasm_module_def& module, module_mappings_t& mappings);
	
	void parse_functions(const wasm_module_def& module, module_mappings_t& mappings);

	void add_start_function(const wasm_module_def& module, module_mappings_t& mappings);

	function_def_t parse_function_body(wasm_binary_parser& parser);

	initializer_expression_t 
	parse_initializer_expression(wasm_binary_parser& parser, const module_mappings_t& mappings) const;

	void finalize_code(wasm_code_string_t& code, module_mappings_t& mappings);

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


	std::size_t resolve_import(const std::string& name, external_kind kind);

	std::size_t add_export(const std::string& name, external_kind kind);
	
	std::size_t add_empty_definition(external_kind kind);
	
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
	
	void resolve_initializers(module_mappings_t& mappings);


	std::unordered_map<std::string, export_t> export_map;
	FunctionSignatureRegistrar sig_registrar;
	std::vector<std::optional<function_def_t>> function_defs;
	std::vector<std::optional<global_variable_def_t>> global_variable_defs;
	std::vector<std::optional<linear_memory_def_t>> linear_memory_defs;
	std::vector<std::optional<table_def_t>> table_defs;
	std::ptrdiff_t program_start_function = -1;
	bool program_main_module_added = false;
	friend class wasm_program_state;
};

#endif /* PARSE_DEFS_FUNCTION_DEFS_H */

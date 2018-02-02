#ifndef PARSE_WASM_MODULE_SECTION_PARSER_H
#define PARSE_WASM_MODULE_SECTION_PARSER_H

#include "wasm_base.h"
#include "parse/leb128/leb128.h"
#include "function/wasm_function.h"


struct wasm_module_section_parser
{
	struct resizable_limits_t {
		wasm_uint32_t initial;
		wasm_uint32_t maximum;
	};

	struct table_type_t {
		wasm_language_type elem_type;
		resizable_limits_t limits;
	};

	enum class external_kind_t {
		FUNCTION,
		TABLE,
		MEMORY,
		GLOBAL
	};

	struct global_type_t {
		wasm_language_type content_type;
		bool mutabality;
	};
	
	struct importable_type_t = {
		const external_kind_t kind;
		union {
			wasm_uint32_t function_index;
			table_type_t table_type;
			resizable_limits_t memory_type;
			global_type_t global_variable;
		};
	};

	struct import_entry_t {
		std::string module_str;
		std::string field_str;
		importable_type import;
	};

	struct export_entry_t {
		std::string field_str;
		external_kind_t kind;
		wasm_uint32_t index;
	};

	
	struct initializer_expression_t {
		using opcode_t = std::underlying_type<wasm_instruction>;
		wasm_function init_expr;
	};
	
	struct global_entry_t {
		wasm_language_type type;
		initializer_expression_t init;
	};

	struct element_segment_t {
		wasm_uint32_t index;
		initializer_expression_t offset;
		wasm_uint32_t num_elem;
	};
	
	wasm_module_section_parser(ConstIt begin_, ConstIt end_):
		begin(begin_), pos(begin_), end(end_)
	{
		
	}


	template <class Type>
	Type get_direct()
	{
		assert_space_for(sizeof(Type));
		Type value;
		std::memcpy(&value, pos, sizeof(value));
		pos += sizeof(value);
		return value;
	}

	template <class Type>
	Type get_direct_endian()
	{
		return le_to_system(get_direct<Type>());
	}

	wasm_uint8_t get_leb128_uint1();
	wasm_uint8_t get_leb128_uint7();
	wasm_uint8_t get_leb128_uint8();
	wasm_uint16_t get_leb128_uint16();
	wasm_uint32_t get_leb128_uint32();
	wasm_uint64_t get_leb128_uint64();

	wasm_sint8_t  get_leb128_sint7();
	wasm_sint8_t  get_leb128_sint8();
	wasm_sint16_t get_leb128_sint16();
	wasm_sint32_t get_leb128_sint32();
	wasm_sint64_t get_leb128_sint64();

	wasm_language_type get_value_type_code();
	wasm_language_type get_block_type();
	wasm_language_type get_elem_type();
	
	external_kind_t get_external_kind();

	func_sig_id_t get_func_type();

	global_type_t get_global_type();

	resizable_limits_t get_resizable_limits();
	
	table_type_t get_table_type();
	
	import_entry_t get_import_entry();
	
	export_entry_t get_export_entry();

	initializer_expression_t get_init_expr();

	global_entry_t get_global_entry();

	template <class DestIt>
	std::size_t get_type_section(DestIt dest)
	{ return read_sequence(dest, get_func_type); }
	
	template <class DestIt>
	std::size_t get_import_section(DestIt dest)
	{ return read_sequence(dest, get_import_entry); }

	template <class DestIt>
	std::size_t get_function_section(DestIt dest)
	{ return read_sequence(dest, get_leb128_uint32); }

	template <class DestIt>
	std::size_t get_table_section(DestIt dest)
	{ return read_sequence(dest, get_table_type); }

	template <class DestIt>
	std::size_t get_memory_section(DestIt dest)
	{ return read_sequence(dest, get_resizable_limits); }

	template <class DestIt>
	std::size_t get_global_section(DestIt dest)
	{ return read_sequence(dest, get_global_entry); }

	template <class DestIt>
	std::size_t get_export_section(DestIt dest)
	{ return read_sequence(dest, get_export_entry); }

	wasm_uint32_t get_start_section()
	{ return get_leb128_uint32(); }
	
	template <class Func>
	CallOnWriteIterator iterator_from_callable(Func&& func) const
	{
		return CallOnWriteIterator(std::forward<Func>(func));
	}

private:
	template <class Func>
	struct CallOnWriteIterator {
		using difference_type = void;
		using value_type = void;
		using pointer = void;
		using reference = void;
		using iterator_category = std::random_access_iterator_tag;
		
		CallOnWriteIterator() = default;
		CallOnWriteIterator(Func fn): func(fn) 
		{ /* EMPTY CTOR */ }

		CallOnWriteIterator& operator++() 
		{ return *this; }

		CallOnWriteIterator& operator++(int) 
		{ return *this; }

		CallOnAssign operator*()
		{ return CallOnAssign(func); }
	private:
		struct CallOnAssign
		{
			CallOnAssign(Func& func): f(func) 
			{ /* EMPTY CTOR */ }

			template <class ValueType>
			CallOnAssign& operator=(ValueType&& value)
			{
				f(std::forward<ValueType>(value));
				return *this;
			}
		private:
			Func& f;
		};
		Func func;
	};
	inline wasm_uint32_t get_count()
	{ return get_leb128_uint32(); }

	template <class DestIt, class T>
	std::size_t read_sequence(DestIt dest, T (wasm_module_section_parser::* memfunc)())
	{
		auto count = get_count();
		for(auto n = count; n > 0; --n)
			*dest++ = this->*memfunc();
		return count;
	}

	void assert_space_for(std::size_t bytes) const;
	std::size_t bytes_remaining(std::size_t bytes) const;
	std::size_t bytes_consumed(std::size_t bytes) const;
	std::size_t bytes_total(std::size_t bytes) const;
	const char* begin;
	const char* pos;
	const char* end;
};



#endif /* PARSE_WASM_MODULE_SECTION_PARSER_H */

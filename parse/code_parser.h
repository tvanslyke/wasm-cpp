#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "wasm_base.h"
#include "wasm_instruction.h"
#include <string>
#include <algorithm>
#include <stack>
#include <sstream>

struct code_parse_error: public std::runtime_error {};

struct wasm_code_parser
{
	using opcode_t = std::underlying_type_t<wasm_instruction>;
	using code_t = std::basic_string<opcode_t>;
	using code_iterator = std::basic_string<opcode_t>::iterator;


	template <class CodeString>
	wasm_code_parser(CodeString&& codestring, const module_mappings_t& module_mappings):
		code(codestring), unbound_labels()
	{
		
	}
	[[nodiscard]] code_t& parse(bool as_function = true) 
	{
		for(auto pos = code.begin(); pos < code.end(); pos = next_opcode(pos))
		{ /* LOOP */ }

		if (unbound_labels.size() > 0)
			throw code_parse_error("Code fragment finished with unbound labels (missing END opcode).");
		else if(as_function and (code.size() > 0) and not (wasm_opcode::END == code.back()))
			throw code_parse_error("Function code does not end with END opcode.");
		
		return code;
	}
private:
	
	code_iterator next_opcode(code_iterator pos)
	{
		using wasm_opcode::wasm_instruction;
		switch(*pos)
		{
		case BLOCK:
			pos = replace_block_signature(pos);
			unbound_labels.push(pos - code.begin());
			break;
		case LOOP:
			pos = replace_block_signature(pos);
		case BR: 	[[fallthrough]]
		case BR_IF:
			++pos;
			pos = replace_leb128_immediate<wasm_uint32_t>(pos);
			break;
		case BR_TABLE:
			pos = replace_leb128_immediate_array<wasm_uint32_t>(pos);
			// one extra varuint32 holds the value of the default case
			pos = replace_leb128_immediate<wasm_uint32_t>(pos);
			break;
		case IF:
			pos = replace_block_signature(pos);
			unbound_labels.push(pos - code.begin());
			break;
		case ELSE:
			pos = bind_top_label(pos);
			unbound_labels.push(pos - code.begin());
			break;
		case END:
			pos = bind_top_label(pos);
			break;
		case I32_CONST:
			pos = replace_leb128_immediate<wasm_sint32_t>(pos);
			break;
		case I64_CONST:
			pos = replace_leb128_immediate<wasm_sint64_t>(pos);
			break;
		case F32_CONST:
			pos = skip_literal_immediate<float>(pos);
			break;
		case F64_CONST:
			pos = skip_literal_immediate<double>(pos);
			break;
		case GET_LOCAL: 	[[fallthrough]]
		case SET_LOCAL: 	[[fallthrough]]
		case TEE_LOCAL: 
			pos = replace_leb128_immediate<wasm_uint32_t>(pos);
			break;
		case GET_GLOBAL:	[[fallthrough]]
		case SET_GLOBAL:
			pos = replace_leb128_immediate<wasm_uint32_t>(pos, &(module_mapping.globals));
			break;
		case CALL:
			pos = replace_leb128_immediate<wasm_uint32_t>(pos, &(module_mapping.functions));
			break;
		case CALL_INDIRECT:
			pos = replace_leb128_immediate<wasm_uint32_t>(pos, &(module_mapping.function_signatures));
			pos = skip_literal_immediate<wasm_uint8_t>(pos);
			break;
		case I32_LOAD:		[[fallthrough]] 
		case I64_LOAD:		[[fallthrough]]
		case F32_LOAD:		[[fallthrough]]
		case F64_LOAD:		[[fallthrough]]
		case I32_LOAD8_S:	[[fallthrough]]
		case I32_LOAD8_U:	[[fallthrough]]
		case I32_LOAD16_S:	[[fallthrough]]
		case I32_LOAD16_U:	[[fallthrough]]
		case I64_LOAD8_S:	[[fallthrough]]
		case I64_LOAD8_U:	[[fallthrough]]
		case I64_LOAD16_S: 	[[fallthrough]]
		case I64_LOAD16_U:	[[fallthrough]]
		case I64_LOAD32_S:	[[fallthrough]]
		case I64_LOAD32_U:	[[fallthrough]]
		case I32_STORE: 	[[fallthrough]]
		case I64_STORE:		[[fallthrough]]
		case F32_STORE:		[[fallthrough]]
		case F64_STORE:		[[fallthrough]]
		case I32_STORE8:	[[fallthrough]]	
		case I32_STORE16:	[[fallthrough]]
		case I64_STORE8:	[[fallthrough]]
		case I64_STORE16:	[[fallthrough]]
		case I64_STORE32:	
			pos = replace_leb128_immediate<wasm_uint32_t>(pos);
			pos = replace_leb128_immediate<wasm_ptr_t>(pos);
			break;
		case GROW_MEMORY:
		case CURRENT_MEMORY:
			pos = skip_literal_immediate<wasm_uint8_t>(pos);
			break;
		default:
			// opcodes without immediate operands
			if (is_not_instruction(*pos))
			{
				// bad opcode :(
				std::ostringstream ss;
				ss << "Invalid opcode, " << std::hex << *pos << ", encountered.";
				throw code_parse_error(ss.str());
			}
			else
				// good opcode :)
				++pos;
		}
		return pos;
	}
	
	code_iterator bind_top_label(code_iterator pos)
	{
		auto index = unbound_lables.top();
		unbound_lables.pop();
		// 'offset' is the value to be copied directly into the string as
		// an immediate
		auto offset = (pos - code.begin()) - index;
		assert(offset < std::numeric_limits<wasm_uint32_t>::max());
		wasm_uint32_t immediate = offset;
		// save the current position of 'pos' as an index
		auto pos_index = pos - code.begin();
		// insert the offset as an immediate
		auto ins_endpos = insert_immediate(code.begin() + index, immediate);
		auto ins_end_index = ins_endpos - code.begin();
		// update 'pos_index' by advancing by the number of entries that
		// were just added to the string
		pos_index += ins_end_index - index;
		return code.begin() + index;
	}

	static bool is_not_instruction(opcode_t oc)
	{
		return wasm_instruction_dne(oc);
	}

	code_iterator replace_block_signature(code_iterator pos)
	{
		return ++pos;
	}

	template <class Integer, class Mapping = std::vector<std::ptrdiff_t>>
	code_iterator replace_leb128_immediate(code_iterator pos, Mapping* mapping = nullptr)
	{
		// replace the leb128 encoded integer of type Integer
		// with the a bit-blit of its decoded system-endianness value 
		auto [value, stop] = leb128_decode<Integer>(pos, code.end());
		if(mapping)
		{
			assert(value > 0);
			assert(std::size_t(value) < mapping->size());
			value = (*mapping)[value];
		}
		return replace_with_immediate(pos, stop, value);
	}

	template <class Type>
	std::pair<Type, code_iterator> extract_literal_immediate(code_iterator pos)
	{
		Type value;
		constexpr std::size_t count = 
			sizeof(Type) / sizeof(opcode_t) + ((sizeof(Type) % sizeof(opcode_t)) > 0);
		std::memcpy(&value, &(*pos), sizeof(value));
		pos += count;
		return {value, pos};
	}

	template <class Type>
	code_iterator skip_literal_immediate(code_iterator pos)
	{
		auto [_, it] = extract_literal_immediate(pos);
		return it;
	}


	template <class Type>
	code_iterator replace_with_immediate(code_iterator begin, code_iterator end, Type value)
	{
		constexpr std::size_t bytecount = 
			(sizeof(value) + (sizeof(value) % sizeof(opcode_t)));
		std::array<opcode_t, bytecount> buff;
		buff.fill(0);
		std::memcpy(buff.data(), &value, sizeof(value));
		std::size_t index = pos - code.begin();
		code.replace(pos, stop, buff.begin(), buff.end());
		return code.begin() + index + buff.size();
	}
	template <class Type>
	code_iterator insert_immediate(code_iterator begin, Type value)
	{
		return replace_with_immediate(begin, begin, value);
	}
	template <class Integer>
	code_iterator replace_leb128_immediate_array(code_iterator pos)
	{
		wasm_uint32_t count = 0;
		std::tie(count, pos) = leb128_decode<wasm_uint32_t>(pos, code.end());
		for(; count > 0; --count)
			pos = replace_leb128_immediate<Integer>(pos);
		return pos;
	}
	code_t code;
	module_mapping_t& module_mapping;
	std::stack<std::size_t> unbound_labels;
};

template <class String>
[[nodiscard]] wasm_code_parser::code_t finalize_code(String&& code, bool as_function = true)
{
	return wasm_code_parser(std::forward<String>(code)).parse(as_function);
}


#endif /* CODE_PARSER_H */

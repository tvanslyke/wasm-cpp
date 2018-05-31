#ifndef BINPARSE_H
#define BINPARSE_H
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <string>
#include <utility>
#include <optional>
#include <variant>
#include <stdexcept>
#include <cassert>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include "../include/WasmInstruction.h"
#include "binparse/types.h"
#include "binparse/rules.h"
#include "binparse/rule_defs.h"


namespace wasm::parse {
namespace x3 = boost::spirit::x3;

enum class SectionType: unsigned char {
	Custom   = 0,
	Type     = 1,
	Import   = 2,
	Function = 3,
	Table    = 4,
	Memory   = 5,
	Global   = 6,
	Export   = 7,
	Start    = 8,
	Element  = 9,
	Code     = 10,
	Data     = 11
};

template <SectionType SecType, class SectionParser>
struct ModuleSectionParser:
	public x3::unary_parser<SectionParser, ModuleSectionParser<SecType, SectionParser>>
{
	using base_type = x3::unary_parser<SectionParser, ModuleSectionParser<SecType, SectionParser>>;
	static constexpr bool is_pass_through_unary = true;
	ModuleSectionParser(SectionParser&& sp):
		base_type(std::move(sp))
	{
		
	}

	ModuleSectionParser(const SectionParser& sp):
		base_type(sp)
	{
		
	}

	template <typename It, typename Ctx, typename Other, typename Attr>
        bool parse(It& f_, It l, Ctx&& ctx, const Other & other, Attr& attr) const 
	{
		auto f = f_;
		if(std::uint8_t id{0}; not varuint7.parse(f, l, ctx, other, id))
		{
			return false;
		}
		else if(id != static_cast<unsigned>(SecType))
		{
			return false;
		}
		std::ptrdiff_t payload_len{0};
		if(not varuint32.parse(f, l, ctx, other, payload_len))
			return false;
		auto dist = std::distance(f, l);
		if(dist < payload_len)
			return false;
		if constexpr(SecType == SectionType::Custom)
		{
			std::advance(f, payload_len);
		}
		else
		{
			auto first = f;
			auto last = std::next(f, payload_len);
			if(not this->subject.parse(f, last, ctx, other, attr))
				return false;
			if(f != last)
				throw expectation_failure<It>(
					f, 
					std::string("Section payload length not honored.  Expected ")
					+ std::to_string(payload_len) 
					+ " bytes consumed but saw only "
					+ std::to_string(std::distance(first, f))
					+ " consumed in section "
					+ std::to_string(static_cast<unsigned>(SecType))
					+ "."
				);
		}
		f_ = f;
		return true;
	}
};

template <SectionType Sec, class Parser>
auto module_section(Parser&& parser)
{
	return ModuleSectionParser<Sec, std::decay_t<Parser>>(
		std::forward<Parser>(parser)
	);
}

namespace codeparse {

using opc::OpCode;

inline const auto append_opcode = [](auto& ctx) {
	unsigned char op = static_cast<unsigned char>(x3::_attr(ctx));
	assert(opc::opcode_exists(op));
	x3::_val(ctx).push_back(op);
};

inline const auto append_serialized = [](auto& ctx) {
	auto value = x3::_attr(ctx);
	static_assert(std::is_arithmetic_v<decltype(value)>);
	auto& str = x3::_val(ctx);
	auto oldsz = str.size();
	str.resize(oldsz + sizeof(value));
	std::memcpy(str.data() + oldsz, &value, sizeof(value));
};

inline const auto append_code = [](auto& ctx) {
	auto& inner_code = x3::_attr(ctx);
	auto& outer_code = x3::_val(ctx);
	outer_code += std::move(inner_code);
};

template <OpCode Op>
inline const auto parse_opcode
	= match_value(x3::byte_, static_cast<unsigned char>(Op), static_cast<char>(Op));

template <class Parser>
auto serialize_native(Parser&& parser)
{
	return as<std::string>[std::forward<Parser>(parser)[append_serialized]];
}

inline const auto varuint32_to_native = serialize_native(varuint32);
inline const auto varint32_to_native  = serialize_native(varint32);
inline const auto varint64_to_native  = serialize_native(varint64);

inline const auto memory_immed
	= x3::rule<struct memory_immed_tag, std::string>{}
	= varuint32_to_native[append_code] >> varuint32_to_native[append_code];

inline const auto block_immed
	= x3::rule<struct block_immed_tag, std::string>{}
	= block_type[(
		[](auto& ctx) {
			char c = static_cast<char>(x3::_attr(ctx));
			x3::_val(ctx).push_back(c);
		}
	)];
	
inline const auto reserved = x3::omit[varuint1];

inline const auto& index_immed = varuint32_to_native;

inline const auto branch_table_immed
	= x3::rule<struct branch_table_immed_tag, std::string>("branch_table_immed")
	= as<std::vector<std::uint32_t>>[
		varuint32_prefixed_sequence(varuint32) >> varuint32
	][([](auto& ctx) {
			auto& str = x3::_val(ctx);
			auto& vec = x3::_attr(ctx);
			auto old_sz = str.size();
			str.resize(vec.size() + 1);
			std::uint32_t table_sz = vec.size() - 1u;
			assert(table_sz == (vec.size() - 1u)); // no truncation
			auto pos = str.data() + old_sz;
			std::memcpy(pos, &table_sz, sizeof(table_sz));
			pos += sizeof(table_sz);
			std::memcpy(pos, vec.data(), vec.size() * sizeof(vec.front()));
		}
	)];


inline const auto br_opcode
	= x3::rule<struct br_opcode_tag, std::string>("br_opcode")
	= (parse_opcode<OpCode::BR>[append_opcode] | parse_opcode<OpCode::BR_IF>[append_opcode])
	> index_immed[append_code];

inline const auto br_table_opcode
	= x3::rule<struct br_table_opcode_tag, std::string>("br_table_opcode")
	= (parse_opcode<OpCode::BR_TABLE>[append_opcode] > branch_table_immed[append_code]);

inline const auto call_opcode
	= x3::rule<struct call_opcode_tag, std::string>("call_opcode")
	= (parse_opcode<OpCode::CALL>[append_opcode] > index_immed[append_code]);

inline const auto call_indirect_opcode 
	= x3::rule<struct call_indirect_opcode_tag, std::string>("call_indirect_opcode")
	= (parse_opcode<OpCode::CALL>[append_opcode] > index_immed[append_code]) > reserved;

inline const auto variable_access_opcode
	= x3::rule<struct variable_access_opcode_tag, std::string>("variable_access_opcode")
	= (
		parse_opcode<OpCode::GET_LOCAL>[append_opcode]
		| parse_opcode<OpCode::SET_LOCAL>[append_opcode]
		| parse_opcode<OpCode::TEE_LOCAL>[append_opcode]
		| parse_opcode<OpCode::GET_GLOBAL>[append_opcode]
		| parse_opcode<OpCode::SET_GLOBAL>[append_opcode]
	)
	> index_immed[append_code];


inline const auto memory_access_opcode
	= x3::rule<struct memory_access_opcode_tag, std::string>("memory_access_opcode")
	= x3::byte_[(
		[](auto& ctx) {
			auto opcd = static_cast<unsigned>(x3::_attr(ctx));
			x3::_pass(ctx) = (opcd >= 0x28u and opcd < 0x3Eu);
		}
	)][append_opcode]
	> memory_immed[append_code];

inline const auto memory_grow_or_query_opcode
	= x3::rule<struct memory_grow_or_query_opcode_tag, std::string>("memory_grow_or_query_opcode")
	= (
		parse_opcode<OpCode::CURRENT_MEMORY>[append_opcode]
		| parse_opcode<OpCode::GROW_MEMORY>[append_opcode]
	)
	> reserved;

inline const auto const_opcode
	= x3::rule<struct const_opcode_tag, std::string>("const_opcode")
	= (
		  (parse_opcode<OpCode::I32_CONST>[append_opcode] > varint32[append_serialized])
		| (parse_opcode<OpCode::I64_CONST>[append_opcode] > varint64[append_serialized])
		| (parse_opcode<OpCode::F32_CONST>[append_opcode] > float32[append_serialized])
		| (parse_opcode<OpCode::F64_CONST>[append_opcode] > float64[append_serialized])
	);


inline const auto normal_opcode
	= x3::rule<struct normal_opcode_tag, char>("normal_opcode")
	= x3::byte_[([](auto& ctx) {
		if(not wasm::opc::opcode_exists(x3::_attr(ctx)))
		{
			x3::_pass(ctx) = false;
			return;
		}
		OpCode op = static_cast<OpCode>(x3::_attr(ctx));
		x3::_pass(ctx) = (
			(op != OpCode::ELSE)
			and (op != OpCode::END)
		);
		x3::_val(ctx) = static_cast<char>(op);
	}
)];

inline const auto loop_opcode
	= x3::rule<struct loop_opcode_tag, std::string>("loop_opcode")
	= parse_opcode<OpCode::LOOP>[append_opcode] > block_immed[append_code];

inline const auto code
	= x3::rule<struct code_tag, std::string>("code");

// an unbound label has the value zero.  This is always invalid since the label must at least
// be '7' to jump over the 'IF', itself, and the 'END' (>7 if there's an 'ELSE')
inline const auto unbound_label
	= x3::attr(std::string(sizeof(std::uint32_t), 0));

inline const auto bind_block_label = [](auto& ctx) {
	auto& str = x3::_val(ctx);
	assert(str.front() == static_cast<char>(OpCode::BLOCK));
	assert(str.size() >= 2u + sizeof(std::uint32_t));
	auto label_pos = str.data() + 2;
	assert(std::all_of(label_pos, label_pos + sizeof(std::uint32_t), [](char c){ return c == 0; }));
	std::uint32_t jumpdist = str.size();
	assert(jumpdist == str.size()); // no truncation
	std::memcpy(label_pos, &jumpdist, sizeof(jumpdist));
};

inline const auto bind_if_else_label = [](auto& ctx) {
	auto& str = x3::_val(ctx);
	assert(str.front() == static_cast<char>(OpCode::IF));
	assert(str.size() >= 3u + 2u * sizeof(std::uint32_t));
	auto label_pos = str.data() + 2;
	assert(std::all_of(label_pos, label_pos + sizeof(std::uint32_t), [](char c){ return c == 0; }));
	assert(std::all_of(str.end() - sizeof(std::uint32_t), str.end(), [](char c){ return c == 0; }));
	assert(str[str.size() - (1u + sizeof(std::uint32_t))] == static_cast<char>(OpCode::ELSE));
	std::uint32_t jumpdist = str.size();
	assert(jumpdist == str.size()); // no truncation
	std::memcpy(label_pos, &jumpdist, sizeof(jumpdist));
};

inline const auto bind_if_end_label = [](auto& ctx) {
	auto& str = x3::_val(ctx);
	assert(str.front() == static_cast<char>(OpCode::IF));
	assert(str.size() >= 3u + sizeof(std::uint32_t));
	auto label_pos = str.data() + 2;
	std::uint32_t jumpdist;
	std::memcpy(&jumpdist, label_pos, sizeof(jumpdist));
	if(jumpdist == 0u)
	{
		jumpdist = str.size();
		std::memcpy(label_pos, &jumpdist, sizeof(jumpdist));
	}
	else
	{
		assert(jumpdist < str.size());
		assert(jumpdist >= 3u + sizeof(std::uint32_t));
		label_pos = str.data() + jumpdist;
		label_pos -= sizeof(std::uint32_t);
		assert(std::all_of(label_pos, label_pos + sizeof(std::uint32_t), [](char c){ return c == 0; }));
		auto else_pos = label_pos - 1;
		assert(*else_pos == static_cast<char>(OpCode::ELSE));
		jumpdist = std::distance(else_pos, std::addressof(*str.end()));
		std::memcpy(label_pos, &jumpdist, sizeof(jumpdist));
	}
};

inline const auto block_opcode
	= x3::rule<struct block_opcode_tag, std::string>{"block_opcode"}
	= (parse_opcode<OpCode::BLOCK>[append_opcode] > block_immed[append_code])
	>> unbound_label[append_code]
	>> code[append_code]
	>> parse_opcode<OpCode::END>[append_opcode][bind_block_label];

inline const auto if_opcode
	= x3::rule<struct if_opcode_tag, std::string>{"if_opcode"}
	= (parse_opcode<OpCode::IF>[append_opcode] > block_immed[append_code])
	>> unbound_label[append_code]
	>> code[append_code]
	>> (-(
		parse_opcode<OpCode::ELSE>[append_opcode] 
		>> unbound_label[append_code][bind_if_else_label]
		>> code[append_code]
	))
	>> parse_opcode<OpCode::END>[append_opcode][bind_if_end_label];

inline const auto code_def =
	*(
		  block_opcode[append_code]
		| if_opcode[append_code]
		| loop_opcode[append_code]
		| br_opcode[append_code]
		| br_table_opcode[append_code]
		| call_opcode[append_code]
		| call_indirect_opcode[append_code]
		| variable_access_opcode[append_code]
		| memory_access_opcode[append_code]
		| memory_grow_or_query_opcode[append_code]
		| const_opcode[append_code]
		| normal_opcode[append_opcode]
	);

BOOST_SPIRIT_DEFINE(code);

inline const auto function_body_code
	= x3::rule<struct function_body_code_tag, std::string>("function_body_code")
	= (code > x3::omit[parse_opcode<OpCode::END>])[(
		[](auto& ctx) {
			assert(x3::_val(ctx).empty());
			x3::_val(ctx) = std::move(x3::_attr(ctx));
			x3::_val(ctx).push_back(static_cast<char>(OpCode::END));
		}
	)];

} /* namespace codeparse */


struct FunctionBodyParser:
	public x3::parser<FunctionBodyParser>
{
	using attribute_type = FunctionBody;
	static constexpr const bool has_attribute = true;

        template <typename It, typename Ctx, typename Other, typename Attr>
        bool parse(It& first_, It last, Ctx&& ctx, Other&& other, Attr& attr) const
	{
		auto first = first;
		std::string body;
		if(
			(not varuint32_prefixed_sequence(x3::char_).parse(first, last, ctx, other, body)) 
			or body.size() == 0u
		)
		{
			return false;
		}
		else if(body.back() != relax_enum(wasm::opc::OpCode::END))
		{
			throw expectation_failure<It>(
				first_ + (body.size() - 1),
				"Function body does not end on an END opcode."
			);
		}
		
		
		std::vector<LocalEntry> local_entries;
		auto parse_locals = varuint32_prefixed_sequence(local_entry);
		auto start = body.begin();
		auto pos = start;
		if(not parse_locals.parse(pos, body.end(), ctx, other, local_entries))
			return false;
		if(pos == body.end())
		{
			throw expectation_failure<It>(
				first_ + (body.size() - 1),
				"Function body too short."
			);
		}
		// get rid of the local entries data
		std::string body_code;
		body_code.reserve(body.size());
		if(not codeparse::function_body_code.parse(
			pos, body.end(), ctx, other, body_code
		))
		{
			return false
		}
		else if(pos != body.end())
		{
			throw expectation_failure<It>(
				first_ + (pos - body.begin()),
				"Function code finished before end of FunctionBody."
			);
		}
		if constexpr(not std::is_same_v<std::decay_t<Attr>, x3::unused_type>)
		{
			assert(attr.locals.empty());
			for(const auto& entry: local_entries)
				attr.locals.append(entry.count, entry.type);
			attr.code = std::move(body_code);
		}
		first_ = first;
		return true;
	}
	
};

inline const auto function_body = FunctionBodyParser{};

inline const auto type_section 
	= x3::rule<struct type_section_tag, std::vector<FunctionSignature>>{"type_section"}
	= varuint32_prefixed_sequence(func_type);

inline const auto import_section 
	= x3::rule<struct import_section_tag, std::vector<ImportEntry>>{"import_entry"}
	= varuint32_prefixed_sequence(import_entry);

inline const auto function_section 
	= x3::rule<struct function_section_tag, std::vector<std::uint32_t>>{"function_section"}
	= varuint32_prefixed_sequence(varuint32);

inline const auto table_section
	= x3::rule<struct table_section_tag, std::vector<Table>>{"table_section"}
	= varuint32_prefixed_sequence(table_type);

inline const auto memory_section
	= x3::rule<struct memory_section_tag, std::vector<Memory>>{"memory_section"}
	= varuint32_prefixed_sequence(memory_type);

inline const auto global_section
	= x3::rule<struct global_section_tag, std::vector<GlobalEntry>>{"global_section"}
	= varuint32_prefixed_sequence(global_entry);

inline const auto export_section
	= x3::rule<struct export_section_tag, std::vector<ExportEntry>>{"export_section"}
	= varuint32_prefixed_sequence(export_entry);

inline const auto start_section
	= x3::rule<struct start_section_tag, std::uint32_t>{"start_section"}
	= varuint32;

inline const auto element_section
	= x3::rule<struct element_section_tag, std::vector<ElemSegment>>{"element_section"}
	= varuint32_prefixed_sequence(elem_segment);

inline const auto code_section
	= x3::rule<struct code_section_tag, std::vector<FunctionBody>>{"code_section"}
	= varuint32_prefixed_sequence(function_body);

inline const auto data_section 
	= x3::rule<struct data_section_tag, std::vector<DataSegment>>{"data_section"}
	= varuint32_prefixed_sequence(data_segment);

inline const auto custom_section
	= x3::rule<struct custom_section_tag>{"custom_section"}
	= x3::eps;

inline const auto module_header
	=  x3::byte_(0) >> x3::byte_('a') >> x3::byte_('s') >> x3::byte_('m')
	>> x3::byte_(1) >> x3::byte_(0)   >> x3::byte_(0)   >> x3::byte_(0);

inline const auto section_id_good = 
	x3::expect[&(
		  x3::byte_(0)
		| x3::byte_(1)
		| x3::byte_(2)
		| x3::byte_(3)
		| x3::byte_(4)
		| x3::byte_(5)
		| x3::byte_(6)
		| x3::byte_(7)
		| x3::byte_(8)
		| x3::byte_(9)
		| x3::byte_(10)
		| x3::byte_(11)
		| x3::eoi
	)];

inline const auto custom_sections
	= *x3::omit[section_id_good >> module_section<SectionType::Custom>(custom_section)];

inline const auto module
	= x3::rule<struct module_tag, ModuleDef>{"module"}
	= x3::omit[module_header]
	>> (
		x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Type>(type_section)         >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Import>(import_section)     >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Function>(function_section) >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Table>(table_section)       >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Memory>(memory_section)     >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Global>(global_section)     >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Export>(export_section)     >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Start>(start_section)       >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Element>(element_section)   >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Code>(code_section)         >> x3::omit[custom_sections]
		>> section_id_good >> -module_section<SectionType::Data>(data_section)         >> x3::omit[custom_sections]
	);

inline const auto module_strict = 
	module >> x3::eps[(
		[](auto& ctx) {
			auto f = x3::_where(ctx).begin();
			auto l = x3::_where(ctx).end();
			if(f != l)
			{
				x3::_pass(ctx) = false;
				throw expectation_failure<std::decay_t<decltype(f)>>(
					f,
					"Module terminates before end of data."
				);
			}
		}
	)];


template <class T, class It>
std::pair<T, It> read_immediate(It first, It last)
{
	static_assert(std::is_arithmetic_v<T>);
	static_assert(
		std::is_same_v<std::uint32_t, T>
		or std::is_same_v<std::uint64_t, T>
		or std::is_same_v<std::int32_t, T>
		or std::is_same_v<std::int64_t, T>
		or std::is_same_v<float, T>
		or std::is_same_v<double, T>
	);
	alignas(T) char buff[sizeof(T)];
	T value;
	assert(std::distance(first, last) >= sizeof(buff));
	for(std::size_t i = 0; i < sizeof(buff); ++i)
		buff[i] = *first++;
	std::memcpy(&value, buff, sizeof(value));
	return std::make_pair(value, first);
}

template <class It>
It write_opcode(std::ostream& os, It first, It last, std::size_t& indent, std::size_t indent_depth, bool _show_labels = false)
{
	using wasm::opc::OpCode;
	if(first == last)
		return first;
	OpCode op;
	{
		auto tmp = *first++;
		assert(opc::opcode_exists(tmp));
		op = static_cast<OpCode>(tmp);
	}
	if(op == OpCode::END)
	{
		assert(indent > 0u);
		--indent;
		os << std::string(indent_depth * indent, ' ') << op;
	}
	else if(op == OpCode::ELSE)
	{
		assert(indent > 0u);
		os << std::string(indent_depth * (indent - 1), ' ') << op;
	}
	else
	{
		os << std::string(indent_depth * indent, ' ') << op;
	}

	// write immediates ... 
	switch(op)
	{
	case OpCode::BLOCK: [[fallthrough]];
	case OpCode::LOOP:  [[fallthrough]];
	case OpCode::IF: {
		assert(first != last);
		LanguageType tp;
		signed char tmp = *first++;
		assert(block_type_exists(tmp));
		tp = static_cast<LanguageType>(tmp);
		os << ' ' << tp;
		++indent;
		break;
	}
	case OpCode::BR:            [[fallthrough]];
	case OpCode::BR_IF:         [[fallthrough]];
	case OpCode::CALL:          [[fallthrough]];
	case OpCode::CALL_INDIRECT: [[fallthrough]];
	case OpCode::GET_LOCAL:     [[fallthrough]];
	case OpCode::SET_LOCAL:     [[fallthrough]];
	case OpCode::TEE_LOCAL:     [[fallthrough]];
	case OpCode::GET_GLOBAL:    [[fallthrough]];
	case OpCode::SET_GLOBAL: {
		std::uint32_t depth;
		std::tie(depth, first) = read_immediate<std::uint32_t>(first, last);
		os << ' ' << depth;
		break;
	}
	case OpCode::BR_TABLE: {
		std::uint32_t count;
		std::uint32_t depth;
		std::tie(count, first) = read_immediate<std::uint32_t>(first, last);
		for(std::uint32_t i = 0; i < count; ++i)
		{
			std::tie(depth, first) = read_immediate<std::uint32_t>(first, last);
			os << ' ' << depth;
		}
		std::tie(depth, first) = read_immediate<std::uint32_t>(first, last);
		os << ' ' << depth;
		break;
	}
	case OpCode::I32_LOAD:     [[fallthrough]];
	case OpCode::I64_LOAD:     [[fallthrough]];
	case OpCode::F32_LOAD:     [[fallthrough]];
	case OpCode::F64_LOAD:     [[fallthrough]];
	case OpCode::I32_LOAD8_S:  [[fallthrough]];
	case OpCode::I32_LOAD8_U:  [[fallthrough]];
	case OpCode::I32_LOAD16_S: [[fallthrough]];
	case OpCode::I32_LOAD16_U: [[fallthrough]];
	case OpCode::I64_LOAD8_S:  [[fallthrough]];
	case OpCode::I64_LOAD8_U:  [[fallthrough]];
	case OpCode::I64_LOAD16_S: [[fallthrough]];
	case OpCode::I64_LOAD16_U: [[fallthrough]];
	case OpCode::I64_LOAD32_S: [[fallthrough]];
	case OpCode::I64_LOAD32_U: [[fallthrough]];
	case OpCode::I32_STORE:    [[fallthrough]];
	case OpCode::I64_STORE:    [[fallthrough]];
	case OpCode::F32_STORE:    [[fallthrough]];
	case OpCode::F64_STORE:    [[fallthrough]];
	case OpCode::I32_STORE8:   [[fallthrough]];
	case OpCode::I32_STORE16:  [[fallthrough]];
	case OpCode::I64_STORE8:   [[fallthrough]];
	case OpCode::I64_STORE16:  [[fallthrough]];
	case OpCode::I64_STORE32:  {
	        std::uint32_t align;
	        std::uint32_t offset;
	        std::tie(align, first) = read_immediate<std::uint32_t>(first, last);
	        align = 1 << align;
	        std::tie(offset, first) = read_immediate<std::uint32_t>(first, last);
	        os << " offset=" << offset << " align=" << align << ' ';
		break;
	}
	case OpCode::I32_CONST: {
		std::int32_t value;
		std::tie(value, first) = read_immediate<std::int32_t>(first, last);
		os << ' ' << value;
		break;
	}
	case OpCode::I64_CONST: {
		std::int64_t value;
		std::tie(value, first) = read_immediate<std::int64_t>(first, last);
		os << ' ' << value;
		break;
	}
	case OpCode::F32_CONST: {
		float value;
		std::tie(value, first) = read_immediate<float>(first, last);
		os << ' ' << value;
		break;
	}
	case OpCode::F64_CONST: {
		double value;
		std::tie(value, first) = read_immediate<double>(first, last);
		os << ' ' << value;
		break;
	}
	default:
		(void)0;
	} /* switch */
	
	// skip over jump distance immediate
	if(op == OpCode::BLOCK or op == OpCode::ELSE or op == OpCode::IF)
	{
		std::uint32_t label;
		std::tie(label, first) = read_immediate<std::uint32_t>(first, last);
		if(_show_labels)
		{
			os << " (; label = " << label << ", (at instruction '";
			if(op == OpCode::BLOCK or op == OpCode::IF)
				label -= (2u + sizeof(std::uint32_t));
			else
				label -= (1 + sizeof(std::uint32_t));
			if(label < std::distance(first, last))
				os << static_cast<OpCode>(*std::next(first, label));
			else
				os << "OUT_OF_RANGE";
			os <<  "');)";
		}
	}
	return first;
}

template <class It>
It write_code(std::ostream& os, It first, It last, std::size_t indent = 0, bool _show_labels = false)
{
	for(std::size_t indent = full_function_body; first != last; )
	{
		first = write_opcode(os, first, last, indent, 2u, _show_labels);
		os << '\n';
	}
	return first;
}

} /* namespace wasm::parse */
#endif /* BINPARSE_H */

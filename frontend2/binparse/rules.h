#ifndef BINPARSE_RULES_H
#define BINPARSE_RULES_H

#include <boost/spirit/home/x3.hpp>
#include "../../include/wasm_base.h"
#include "helpers.h"

namespace wasm::parse {

inline const auto value_type
	= x3::rule<struct value_type_tag, LanguageType>("value_type");

inline const auto block_type
	= x3::rule<struct block_type_tag, LanguageType>("block_type");

inline const auto language_type
	= x3::rule<struct language_type_tag, LanguageType>("language_type");

inline const auto utf8_string
	= x3::rule<struct utf8_string_tag, std::string>("utf8_string");

inline const auto global_type = 
	x3::rule<struct elem_type_tag, GlobalType>("global_type");

inline const auto elem_type
	= x3::rule<struct elem_type_tag, LanguageType>("elem_type");

inline const auto resizable_limits
	= x3::rule<struct resizable_limits_tag, ResizableLimits>("resizable_limits");

inline const auto table_type = x3::omit[elem_type] >> as<Table>[resizable_limits];

inline const auto memory_type = as<Memory>[resizable_limits];

namespace external_kind {
	inline const auto Function = match_value<unsigned char>(x3::byte_, ExternalKind::Function);
	inline const auto Table    = match_value<unsigned char>(x3::byte_, ExternalKind::Table);
	inline const auto Memory   = match_value<unsigned char>(x3::byte_, ExternalKind::Memory);
	inline const auto Global   = match_value<unsigned char>(x3::byte_, ExternalKind::Global);
	inline const auto AnyKind = Function | Table | Memory | Global;
}

inline const auto func_type
	= x3::rule<struct func_type_tag, FunctionSignature>("func_type");

using initializer_t = std::variant<
	std::int32_t,
	std::int64_t,
	float,
	double,
	std::uint32_t
>;

inline const auto float32
	= x3::rule<struct float32_tag, float>("float32");
	
inline const auto float64
	= x3::rule<struct float64_tag, double>("float64");

inline const auto initializer_expression = 
	x3::rule<
		struct initializer_expression_tag,
		initializer_t
	>("initializer_expression");

inline const auto i32_initializer_expression = 
	x3::rule<
		struct i32_initializer_expression_tag,
		std::variant<std::int32_t, std::uint32_t>
	>("i32_initializer_expression");

inline const auto global_entry =
	x3::rule<struct global_entry_tag, GlobalEntry>("global_entry");

inline const auto export_entry
	= x3::rule<struct export_entry_tag, ExportEntry>("export_entry");

inline const auto elem_segment 
	= x3::rule<struct elem_segment_tag, ElemSegment>("elem_segment");

inline const auto data_segment 
	= x3::rule<struct elem_segment_tag, DataSegment>("data_segment");

inline const auto local_entry
	= x3::rule<struct local_entry_tag, LocalEntry>("local_entry");

inline const auto import_entry
	= x3::rule<struct import_entry_tag, ImportEntry>("import_entry");


} /* namespace wasm::parse */

#endif /* BINPARSE_RULES_H */

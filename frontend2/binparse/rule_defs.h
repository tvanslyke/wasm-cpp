#ifndef BINPARSE_RULE_DEFS_H
#define BINPARSE_RULE_DEFS_H

#include "rules.h"
#include "helpers.h"
#include "leb128_parsers.h"
#include "../../include/wasm_base.h"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>

namespace wasm::parse {

inline const auto value_type_def
	= match_value(x3::byte_, 0x7f, LanguageType::i32)
	| match_value(x3::byte_, 0x7e, LanguageType::i64)
	| match_value(x3::byte_, 0x7d, LanguageType::f32)
	| match_value(x3::byte_, 0x7c, LanguageType::f64);
BOOST_SPIRIT_DEFINE(value_type);

inline const auto block_type_def
	= value_type | match_value(x3::byte_, 0x40, LanguageType::block);
BOOST_SPIRIT_DEFINE(block_type);

inline const auto language_type_def
	= block_type
	| match_value(x3::byte_, 0x70, LanguageType::anyfunc)
	| match_value(x3::byte_, 0x60, LanguageType::func);
BOOST_SPIRIT_DEFINE(language_type);

inline const auto utf8_string_def
	= varuint32_prefixed_sequence(x3::char_);
BOOST_SPIRIT_DEFINE(utf8_string);

inline const auto global_type_def
	= value_type >> varuint1;
BOOST_SPIRIT_DEFINE(global_type);

inline const auto elem_type_def
	= match_value<unsigned char>(x3::byte_, LanguageType::anyfunc);
BOOST_SPIRIT_DEFINE(elem_type);

inline const auto resizable_limits_def
	= (x3::omit[x3::byte_(0)] >> varuint32 >> x3::attr(std::optional<std::uint32_t>(std::nullopt)))
	| (x3::omit[x3::byte_(1)] >> varuint32 >> varuint32);

BOOST_SPIRIT_DEFINE(resizable_limits);

inline const auto func_type_def
	= x3::omit[x3::byte_(0x60)]
	> varuint32_prefixed_sequence(value_type)
	> varuint1_option(value_type);
BOOST_SPIRIT_DEFINE(func_type);

inline const auto float32_def
	= x3::omit[x3::dword[(
		[](auto& ctx){
			float result;
			auto value = x3::_attr(ctx);
			static_assert(sizeof(result) == sizeof(value));
			std::memcpy(&result, &value, sizeof(result));
			x3::_val(ctx) = result;
		}
	)]];
BOOST_SPIRIT_DEFINE(float32);
	
inline const auto float64_def
	= x3::omit[x3::qword[(
		[](auto& ctx){
			double result;
			auto value = x3::_attr(ctx);
			static_assert(sizeof(result) == sizeof(value));
			std::memcpy(&result, &value, sizeof(result));
			x3::_val(ctx) = result;
		}
	)]];
BOOST_SPIRIT_DEFINE(float64);

inline const auto initializer_expression_def
	= (
		  (x3::byte_(relax_enum(wasm::opc::OpCode::I32_CONST))  >> varint32)
		| (x3::byte_(relax_enum(wasm::opc::OpCode::I64_CONST))  >> varint64)
		| (x3::byte_(relax_enum(wasm::opc::OpCode::F32_CONST))  >> float32)
		| (x3::byte_(relax_enum(wasm::opc::OpCode::F64_CONST))  >> float64)
		| (x3::byte_(relax_enum(wasm::opc::OpCode::GET_GLOBAL)) >> varuint32)
	);
BOOST_SPIRIT_DEFINE(initializer_expression);

inline const auto i32_initializer_expression_def
	= (
		  (x3::byte_(relax_enum(wasm::opc::OpCode::I32_CONST))  >> varint32)
		| (x3::byte_(relax_enum(wasm::opc::OpCode::GET_GLOBAL)) >> varuint32)
	);
BOOST_SPIRIT_DEFINE(i32_initializer_expression);


inline const auto global_entry_def
	= (value_type >> varuint1 >> initializer_expression)[(
		[](auto& ctx) {
			// auto [kind, is_const, expr] = x3::_attr(ctx);
			auto& attribute = x3::_attr(ctx);
			auto kind = boost::fusion::at_c<0>(attribute);
			auto is_const = boost::fusion::at_c<1>(attribute);
			auto expr = boost::fusion::at_c<2>(attribute);
			assert(static_cast<int>(kind) <= -4 and static_cast<int>(kind) >= -1);
			auto& entry = x3::_val(ctx);
			entry.value.is_const = is_const;
			// indicates that the initializer expression depends on the value
			// of another global.
			bool deferred = std::holds_alternative<std::uint32_t>(expr);
			if(deferred)
				entry.depends = std::get<std::uint32_t>(expr);
			else
				entry.depends = std::nullopt;

			switch(kind)
			{
			case LanguageType::i32:
				entry.value.value = (//.template emplace<std::int32_t>(
					deferred ? std::int32_t{} : std::get<std::int32_t>(expr)
				);
				break;
			case LanguageType::i64:
				entry.value.value = (//.template emplace<std::int64_t>(
					deferred ? std::int64_t{} : std::get<std::int64_t>(expr)
				);
				break;
			case LanguageType::f32:
				entry.value.value = (//.template emplace<float>(
					deferred ? float{} : std::get<float>(expr)
				);
				break;
			case LanguageType::f64:
				entry.value.value = (//.template emplace<double>(
					deferred ? double{} : std::get<double>(expr)
				);
				break;
			default:
				assert(false);
			}
		}
	)];
BOOST_SPIRIT_DEFINE(global_entry);

inline const auto export_entry_def
	= utf8_string >> external_kind::AnyKind >> varuint32;
BOOST_SPIRIT_DEFINE(export_entry);

inline const auto elem_segment_def
	= varuint32 >> i32_initializer_expression >> varuint32_prefixed_sequence(varuint32);
BOOST_SPIRIT_DEFINE(elem_segment);

inline const auto data_segment_def
	= varuint32 >> i32_initializer_expression >> varuint32_prefixed_sequence(x3::byte_);
BOOST_SPIRIT_DEFINE(data_segment);

inline const auto local_entry_def
	= varuint32 >> value_type;
BOOST_SPIRIT_DEFINE(local_entry);

inline const auto import_entry_def
	= utf8_string >> utf8_string >> (
		  (x3::omit[external_kind::Function] > varuint32)
		| (x3::omit[external_kind::Table]    > table_type)
		| (x3::omit[external_kind::Memory]   > memory_type)
		| (x3::omit[external_kind::Global]   > global_type)
	);
BOOST_SPIRIT_DEFINE(import_entry);



} /* namespace wasm::parse */

#endif /* BINPARSE_RULE_DEFS_H */

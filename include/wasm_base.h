#ifndef WASM_BASE_H
#define WASM_BASE_H

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <climits>
#include <limits>
#include <array>
namespace wasm {
static_assert(CHAR_BIT == 8, "8-bit bytes are required");
static_assert(std::numeric_limits<double>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(double) == 8, "64-bit floating point values are required.");
static_assert(std::numeric_limits<float>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(float) == 4, "32-bit floating point values are required.");
static_assert(sizeof(std::uint_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::int_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::uint_least64_t) == 8, "64-bit integer support is required.");
static_assert(sizeof(std::int_least64_t) == 8, "32-bit integer support is required.");

using wasm_sint32_t 	= std::int32_t;
using wasm_uint32_t 	= std::uint32_t;
using wasm_sint64_t 	= std::int64_t;
using wasm_uint64_t 	= std::uint64_t;
using wasm_int32_t 	= wasm_sint32_t;
using wasm_int64_t 	= wasm_sint64_t;
using wasm_float32_t 	= float;
using wasm_float64_t 	= double;
using wasm_byte_t    	= char;
using wasm_ubyte_t    	= unsigned char;
using wasm_sbyte_t    	= signed char;
using wasm_size_t    	= std::size_t;

template <class Enum, class = std::enable_if_t<std::is_enum_v<Enum>>>
auto relax_enum(Enum en) -> std::underlying_type_t<Enum>
{ return static_cast<std::underlying_type_t<Enum>>(en); }

enum class LanguageType:
	wasm_sbyte_t
{
	i32 = -0x01,
	i64 = -0x02,
	f32 = -0x03,
	f64 = -0x04,
	anyfunc = -0x10,
	func = -0x20,
	block = -0x40
};

bool value_type_exists(wasm_sbyte_t v)
{ return (v >= -0x01 or v <= -0x04); }

bool block_type_exists(wasm_sbyte_t v)
{ return value_type_exists(v) or v == -0x40; }

bool language_type_exists(wasm_sbyte_t v)
{ return block_type_exists(v) or v == -0x10 or v == -0x20; }

std::ostream& operator<<(std::ostream& os, LanguageType lt)
{
	switch(lt)
	{
	case LanguageType::i32:
		return os << "i32";
	case LanguageType::i64:
		return os << "i64";
	case LanguageType::f32:
		return os << "f32";
	case LanguageType::f64:
		return os << "f64";
	case LanguageType::anyfunc:
		return os << "(; anyfunc ;)";
	case LanguageType::func:
		return os << "(; func ;)";
	case LanguageType::block:
		return os << "(; block ;)";
	default:
		assert(false);
		return os;
	}
}

enum class ExternalKind:
	wasm_ubyte_t
{
	Function = 0,
	Table    = 1,
	Memory   = 2,
	Global   = 3
};


std::ostream& operator<<(std::ostream& os, ExternalKind lt)
{
	assert(unsigned(lt) < 4u);
	return os << (
		(const char*[]){"Function", "Table", "Memory", "Global"}[int(lt)]
	);
}

} /* namespace wasm */

#endif /* WASM_BASE_H */

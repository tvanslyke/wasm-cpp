#ifndef WASM_BASE_H
#define WASM_BASE_H

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <climits>
#include <limits>
#include <array>

namespace wasm {

static_assert(CHAR_BIT == 8, "8-bit 'char's are required");
static_assert(std::numeric_limits<double>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(double) == 8, "64-bit floating point values are required.");
static_assert(std::numeric_limits<float>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(float) == 4, "32-bit floating point values are required.");
static_assert(sizeof(std::uint_least8_t) == 1, "8-bit integer support is required.");
static_assert(sizeof(std::int_least8_t) == 1, "8-bit integer support is required.");
static_assert(sizeof(std::uint_least16_t) == 2, "16-bit integer support is required.");
static_assert(sizeof(std::int_least16_t) == 2, "16-bit integer support is required.");
static_assert(sizeof(std::uint_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::int_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::uint_least64_t) == 8, "64-bit integer support is required.");
static_assert(sizeof(std::int_least64_t) == 8, "32-bit integer support is required.");

namespace detail {

inline constexpr const bool system_is_twos_complement = (-int(1)) == (~int(0));
inline const bool system_is_little_endian = [](){
	std::uintmax_t one = 1u;
	alignas(std::uintmax_t) char buff[sizeof(std::uintmax_t)];
	std::memcpy(buff, &one, sizeof(one));
	return buff[0] == 1;
}();
} /* namespace detail */

using wasm_sint8_t   = std::int8_t;
using wasm_uint8_t   = std::uint8_t;
using wasm_sint16_t  = std::int16_t;
using wasm_uint16_t  = std::uint16_t;
using wasm_sint32_t  = std::int32_t;
using wasm_uint32_t  = std::uint32_t;
using wasm_sint64_t  = std::int64_t;
using wasm_uint64_t  = std::uint64_t;
using wasm_float32_t = float;
using wasm_float64_t = double;
using wasm_byte_t    = char;
using wasm_ubyte_t   = unsigned char;
using wasm_sbyte_t   = signed char;
using wasm_size_t    = std::size_t;

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

/// @name Traits
/// @{

template <class T>
struct language_type_value;

template <>
struct language_type_value<wasm_sint32_t>: 
	public std::integral_constant<LanguageType, LanguageType::i32>
{}

template <>
struct language_type_value<wasm_sint64_t>: 
	public std::integral_constant<LanguageType, LanguageType::i64>
{}

template <>
struct language_type_value<wasm_float32_t>: 
	public std::integral_constant<LanguageType, LanguageType::f32>
{}

template <>
struct language_type_value<wasm_float64_t>: 
	public std::integral_constant<LanguageType, LanguageType::f64>
{}

template <class T>
inline constexpr const LanguageType language_type_value_v = language_type_value<T>::value;

template <LanguageType Tp>
struct language_type_type;

template <>
struct language_type_type<LanguageType::i32> { using type = wasm_sint32_t; };

template <>
struct language_type_type<LanguageType::i64> { using type = wasm_sint64_t; };

template <>
struct language_type_type<LanguageType::f32> { using type = wasm_float32_t; };

template <>
struct language_type_type<LanguageType::f64> { using type = wasm_float64_t; };

/// @} Traits

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

template <class Exc>
struct ValidationError:
	public Exc
{
	using base_type = Exc;
	using base_type::base_type;
};

} /* namespace wasm */

#endif /* WASM_BASE_H */

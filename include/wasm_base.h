#ifndef WASM_BASE_H
#define WASM_BASE_H

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <climits>
#include <limits>

static_assert(CHAR_BIT == 8, "8-bit bytes are required");
static_assert(std::numeric_limits<double>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(double) == 8, "64-bit floating point values are required.");
static_assert(std::numeric_limits<float>::is_iec559, "IEE 754 is required.");
static_assert(sizeof(float) == 4, "32-bit floating point values are required.");
static_assert(sizeof(std::uint_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::int_least32_t) == 4, "32-bit integer support is required.");
static_assert(sizeof(std::uint_least64_t) == 8, "64-bit integer support is required.");
static_assert(sizeof(std::int_least64_t) == 8, "32-bit integer support is required.");

using wasm_sint32_t 	= std::int_least32_t;
using wasm_uint32_t 	= std::uint_least32_t;
using wasm_sint64_t 	= std::int_least64_t;
using wasm_uint64_t 	= std::uint_least64_t;
using wasm_int32_t 	= wasm_uint32_t;
using wasm_int64_t 	= wasm_uint64_t;
using wasm_float32_t 	= float;
using wasm_float64_t 	= double;
using wasm_byte_t    	= unsigned char;
using wasm_size_t    	= std::size_t;
using wasm_ptr_t	= wasm_uint32_t;

using wasm_sint8_t  	= std::int_least8_t;
using wasm_uint8_t  	= std::uint_least8_t;
using wasm_sint16_t 	= std::int_least16_t;
using wasm_uint16_t 	= std::uint_least16_t;

enum class wasm_language_type: 
	std::int_least8_t
{
	i32 = -0x01,
	i64 = -0x02,
	f32 = -0x03,
	f64 = -0x04,
	anyfunc = -0x10,
	func = -0x20,
	block = -0x40
};



#endif /* WASM_BASE_H */

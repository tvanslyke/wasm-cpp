#ifndef WASM_BASE_H
#define WASM_BASE_H
#include <cstdint>
#include <cstddef>
#include <climits>

static_assert(CHAR_BIT == 8);
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

using wasm_sint32_t = std::int_least32_t;
using wasm_uint32_t = std::uint_least32_t;
using wasm_sint64_t = std::int_least32_t;
using wasm_uint64_t = std::uint_least32_t;
using wasm_int32_t = wasm_uint32_t;
using wasm_int64_t = wasm_uint64_t;
using wasm_float32_t = float;
using wasm_float64_t = double;
using wasm_byte_t = std::uint_least8_t;
using wasm_size_t = std::size_t;




#endif /* WASM_BASE_H */

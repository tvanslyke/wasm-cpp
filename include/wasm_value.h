#ifndef WASM_VALUE_H
#define WASM_VALUE_H

# ifdef __cplusplus
#  include "wasm_base.h"
extern "C" {
# else 
#  include <stdint.h>
typedef uint_least32_t wasm_uint32_t;
typedef uint_least64_t wasm_uint64_t;
typedef int_least32_t  wasm_sint32_t;
typedef int_least64_t  wasm_sint64_t;
typedef float 	       wasm_float32_t;
typedef double 	       wasm_float64_t;
# endif /* __cplusplus */

typedef struct { 
	union// wasm_value_
	{
		wasm_uint32_t  u32;
		wasm_uint64_t  u64;
		wasm_sint32_t  s32;
		wasm_sint64_t  s64;
		wasm_float32_t f32;
		wasm_float64_t f64;
	};
} wasm_value_t;

typedef wasm_value_t WasmValue;

std::ostream& operator<<(std::ostream& os, WasmValue value)
{
	wasm_sint32_t i32;
	wasm_sint64_t i64;
	wasm_float32_t f32;
	wasm_float64_t f64;
	std::memcpy(&i32, &value, sizeof(i32));
	std::memcpy(&i64, &value, sizeof(i64));
	std::memcpy(&f32, &value, sizeof(f32));
	std::memcpy(&f64, &value, sizeof(f64));
	os << "Value(";
	os << "i32 = " << i32;
	os << ", i64 = " << i32;
	os << ", f32 = " << f32;
	os << ", f64 = " << f64;
	os << ')';
	return os;
}

# ifdef __cplusplus

} /* extern "C" */

inline const wasm_value_t& zero_wasm_value() 
{
	static const wasm_value_t zero([](){
		wasm_value_t zer;
		char* mem = reinterpret_cast<char*>(&zer);
		for(std::ptrdiff_t i = 0; i < std::ptrdiff_t(sizeof(zer)); ++i)
			mem[i] = 0;
		return zer;
	}());
	return zero;
}

// pointers-to-members for abstraction over member access
inline wasm_uint32_t  wasm_value_t::* const u_32 = &wasm_value_t::u32;
inline wasm_uint64_t  wasm_value_t::* const u_64 = &wasm_value_t::u64;
inline wasm_sint32_t  wasm_value_t::* const s_32 = &wasm_value_t::s32;
inline wasm_sint64_t  wasm_value_t::* const s_64 = &wasm_value_t::s64;
inline wasm_float32_t wasm_value_t::* const f_32 = &wasm_value_t::f32;
inline wasm_float64_t wasm_value_t::* const f_64 = &wasm_value_t::f64;

# endif /* __cplusplus */
#endif /* WASM_VALUE_H */

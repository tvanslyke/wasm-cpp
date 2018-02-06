#ifndef WASM_VALUE_H
#define WASM_VALUE_H

#ifdef __cplusplus
# include "wasm_base.h"
extern "C" {
#else 
# include <stdint.h>
typedef uint_least32_t wasm_uint32_t;
typedef int_least32_t  wasm_sint32_t;
typedef uint_least64_t wasm_uint32_t;
typedef int_least32_t  wasm_sint32_t;
typedef float 	       wasm_float32_t;
typedef double 	       wasm_float64_t;
#endif 

typedef union wasm_value_
{
	wasm_uint32_t  u32;
	wasm_sint32_t  s32;
	wasm_uint64_t  u64;
	wasm_sint64_t  s64;
	wasm_float32_t f32;
	wasm_float64_t f64;
} wasm_value_t;


#ifdef __cplusplus

} /* extern "C" */

static wasm_uint32_t wasm_value_t::* u_32 = &wasm_value_t::u32;
static wasm_uint64_t wasm_value_t::* u_64 = &wasm_value_t::u64;
static wasm_sint32_t wasm_value_t::* s_32 = &wasm_value_t::s32;
static wasm_sint64_t wasm_value_t::* s_64 = &wasm_value_t::s64;
static wasm_float32_t wasm_value_t::* f_32 = &wasm_value_t::f32;
static wasm_float64_t wasm_value_t::* f_64 = &wasm_value_t::f64;

#endif
#endif /* WASM_VALUE_H */

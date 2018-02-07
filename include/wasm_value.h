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

typedef union wasm_value_
{
	wasm_uint32_t  u32;
	wasm_uint64_t  u64;
	wasm_sint32_t  s32;
	wasm_sint64_t  s64;
	wasm_float32_t f32;
	wasm_float64_t f64;
} wasm_value_t;


# ifdef __cplusplus

} /* extern "C" */

// pointers-to-members for abstraction over member access
extern wasm_uint32_t  wasm_value_t::* const u_32;
extern wasm_uint64_t  wasm_value_t::* const u_64;
extern wasm_sint32_t  wasm_value_t::* const s_32;
extern wasm_sint64_t  wasm_value_t::* const s_64;
extern wasm_float32_t wasm_value_t::* const f_32;
extern wasm_float64_t wasm_value_t::* const f_64;

# endif /* __cplusplus */
#endif /* WASM_VALUE_H */

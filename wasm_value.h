#ifndef WASM_VALUE_H
#define WASM_VALUE_H
#include "wasm_base.h"

union wasm_value_t
{
	wasm_uint32_t  u32;
	wasm_sint32_t  i32;
	wasm_uint64_t  u64;
	wasm_sint64_t  i64;
	wasm_float32_t f32;
	wasm_float64_t f64;
	wasm_value_t*  addr;
};

#endif /* WASM_VALUE_H */

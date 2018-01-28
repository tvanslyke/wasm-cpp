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
};


static const wasm_uint32_t wasm_value_t::* u_32 = &wasm_value_t::u32;
static const wasm_uint64_t wasm_value_t::* u_64 = &wasm_value_t::u64;
static const wasm_sint32_t wasm_value_t::* s_32 = &wasm_value_t::s32;
static const wasm_sint64_t wasm_value_t::* s_64 = &wasm_value_t::s64;
static const wasm_float32_t wasm_value_t::* f_32 = &wasm_value_t::f32;
static const wasm_float64_t wasm_value_t::* f_64 = &wasm_value_t::f64;

#endif /* WASM_VALUE_H */

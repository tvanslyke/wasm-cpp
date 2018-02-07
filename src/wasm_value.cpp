#include "wasm_value.h"

wasm_uint32_t  wasm_value_t::* const u_32 = &wasm_value_t::u32;
wasm_uint64_t  wasm_value_t::* const u_64 = &wasm_value_t::u64;
wasm_sint32_t  wasm_value_t::* const s_32 = &wasm_value_t::s32;
wasm_sint64_t  wasm_value_t::* const s_64 = &wasm_value_t::s64;
wasm_float32_t wasm_value_t::* const f_32 = &wasm_value_t::f32;
wasm_float64_t wasm_value_t::* const f_64 = &wasm_value_t::f64;

#ifndef WASM_TYPES_H
#define WASM_TYPES_H

enum wasm_type_code
{
	wasm_i32_v = 0x7f,
	wasm_i64_v = 0x7e,
	wasm_f32_v = 0x7d,
	wasm_f64_v = 0x7c,
	wasm_anyfunc_v = 0x70,
	wasm_func_v = 0x60,
	wasm_block_v = 0x40
};


#endif /* WASM_TYPES_H */

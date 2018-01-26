#ifndef WASM_MODULE_H
#define WASM_MODULE_H
#include "wasm_base.h"

struct wasm_module
{



	union magic_cookie_t{ 
		wasm_uint32_t value = 0x647;
		wasm_byte_t bytes[4];
	};
	const magic_cookie_t magic_cookie;
	const wasm_uint32_t = 0x01;
		
};

#endif /* WASM_MODULE_H */ 

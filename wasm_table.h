#ifndef WASM_TABLE_H
#define WASM_TABLE_H
#include "wasm_base.h"


struct wasm_table {
	
	virtual void access_at(std::size_t index, void* runtime_state);
};

struct wasm_function_table {
	
	virtual void access_at(std::size_t index, void* runtime_state);
};




#endif /* WASM_TABLE_H */

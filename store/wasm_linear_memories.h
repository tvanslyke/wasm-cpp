#ifndef WASM_LINEAR_MEMORIES_H
#define WASM_LINEAR_MEMORIES_H

#include <vector>
#include "wasm_base.h"
#include "wasm_linear_memory.h"
#include "parse/wasm_module_def.h"
#include "module/wasm_linear_memories.h"

struct wasm_linear_memory_def
{
	wasm_resizable_limits limits;
};

struct wasm_linear_memories
{
	wasm_linear_memory& get_memory(std::size_t index);
	std::size_t add_linear_memory_definition(const wasm_linear_memory_def& def);
private:
	std::vector<wasm_linear_memory> memories;
};


#endif /* WASM_LINEAR_MEMORIES_H */

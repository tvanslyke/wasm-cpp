#ifndef WASM_LINEAR_MEMORY_H
#define WASM_LINEAR_MEMORY_H

#include "wasm_base.h"
#include <array>
#include <vector>


struct wasm_linear_memory
{
	wasm_linear_memory() = default;
	
	static constexpr const wasm_size_t page_size = 64 * 1024;
	void grow_memory(wasm_size_t sz)
	{
		memory.resize(memory.size() + sz * page_size);
	}
	wasm_size_t current_memory() const
	{
		return memory.size() / page_size;
	}
	std::vector<wasm_byte_t> memory;
};

#endif /* WASM_LINEAR_MEMORY_H */

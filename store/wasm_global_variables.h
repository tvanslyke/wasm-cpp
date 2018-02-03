#ifndef WASM_GLOBAL_VARIABLES_H
#define WASM_GLOBAL_VARIABLES_H

#include <vector>
#include "wasm_base.h"
#include "parse/wasm_module_def.h"

struct wasm_global_def
{
	wasm_value_t value;
	bool mutability;
};

struct wasm_globals
{
	wasm_value_t get_global(std::size_t index);
	void set_global(std::size_t index, wasm_value_t value);
	std::size_t add_global_definition(const wasm_global_def& def);
private:
	std::vector<bool> mutabilities;
	std::vector<wasm_value_t> globals;
	// TODO: use a more compact repr for the types
	std::vector<wasm_language_type> types;
};


#endif /* WASM_GLOBAL_VARIABLES_H */

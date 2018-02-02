#ifndef WASM_GLOBALS_H
#define WASM_GLOBALS_H

#include <vector>
#include "wasm_base.h"
#include "wasm_function.h"


struct wasm_globals
{
	wasm_value_t get_global(std::size_t index);
	void set_global(std::size_t index, wasm_value_t value);
	std::size_t add_global_definition(wasm_function&& initializer);
private:
	std::vector<bool> mutabilities;
	std::vector<wasm_value_t> globals;
	// TODO: use a more compact repr for the types
	std::vector<wasm_language_type> types;
	std::vector<wasm_function> init_exprs;
};

	


#endif /* WASM_GLOBALS_H */

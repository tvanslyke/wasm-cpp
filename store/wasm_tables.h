#ifndef WASM_TABLES_H
#define WASM_TABLES_H

#include <vector>
#include "wasm_base.h"
#include "parse/wasm_module_def.h"
#include "module/wasm_table.h"

struct wasm_table_def
{
	wasm_language_type tp;
	wasm_resizable_limits limits;
};

struct wasm_tables
{
	wasm_linear_memory& get_table(std::size_t index);
	std::size_t add_table_definition(const wasm_table_def& def);
private:
	std::vector<std::unique_ptr<wasm_table>> tables;
};


#endif /* WASM_TABLES_H */

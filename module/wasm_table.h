#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include "wasm_base.h"
#include "wasm_value.h"
#include "function/wasm_function.h"


struct wasm_table_base
{
	wasm_table_base() = default;
	virtual ~wasm_table_base() = default;
	virtual wasm_language_type element_type() const = 0;
	virtual void access_indirect(std::size_t index) = 0;
};

struct wasm_anyfunc_table: public wasm_table_base
{

	virtual ~wasm_anyfunc_table() = default;
	virtual wasm_language_type element_type() const final override
	{ return wasm_language_type::anyfunc; } 
	virtual void access_indirect(std::size_t index) const final override
	{
		
	}
private:	
	std::vector<std::shared_pointer<wasm_function>> functions;
};


#endif /* MODULE_WASM_TABLE_H */

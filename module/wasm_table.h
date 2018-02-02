#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include "wasm_base.h"
#include "wasm_value.h"
#include "function/wasm_function.h"
#include "module/wasm_resizable_limits.h"

struct wasm_table_type {
	const wasm_language_type element_type;
	const wasm_resizable_limits limits;
};

bool operator==(const wasm_table_type& left, 
		const wasm_table_type& right)
{ 
	return (left.element_type == other.element_type) 
		and (left.limits == right.limits);
}

bool operator!=(const wasm_table_type& left, 
		const wasm_table_type& right)
{
	return not (left == right);
}

struct wasm_table
{
	wasm_table() = delete;
	wasm_table(wasm_table_type tp): type(tp)
	{ /* EMPTY CTOR */ }

	virtual ~wasm_table() = default;
	virtual void access_indirect(std::size_t index) = 0;
	const wasm_table_type type;
};

struct wasm_anyfunc_table: public wasm_table
{

	~wasm_anyfunc_table() override = default;

	virtual void access_indirect(std::size_t index, ) const final override
	{
		
	}
private:
	std::vector<wasm_function*> functions;

};


#endif /* MODULE_WASM_TABLE_H */

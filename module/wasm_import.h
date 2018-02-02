#ifndef MODULE_WASM_MODULE_IMPORT_H
#define MODULE_WASM_MODULE_IMPORT_H

#include "wasm_base.h"
#include "function/wasm_function.h"

struct wasm_module_export;
struct wasm_module_import;

enum class external_kind {
	function = 0,
	table = 1,
	memory = 2,
	global = 3
};


struct wasm_module_shared_field
{
	
protected:
	wasm_module& module;
	const wasm_language_type kind;
	const std::string field_str;
};

bool operator<(const wasm_module_shared_field& left,
		const wasm_module_shared_field& right);

struct wasm_module_import:
	public wasm_module_shared_field
{


private:
	using import_decl_t = std::variant<
		func_sig_id_t, 				// type (signature) of the function import
		wasm_table_type, 			// type of the table import
		wasm_resizable_limits, 			// type of the memory import
		std::pair<wasm_lanugage_type, bool> 	// type of the global variable import
	>;
	const std::size_t import_offset;
	const import_decl_t import_type;
};


struct wasm_module_export:
	public wasm_module_import_export_base
{
	bool satisfies(const wasm_module_import& import) const
	{
		return kind == other.kind 
			and module.name == other.module.name
			and field_str == other.field_str;
	}
	
	void export_to(wasm_module_import& import)
	{
		assert(this->satisfies(import));
		switch(kind)
		{
		case external_kind::function:
			break;
		case external_kind::table:
			break;
		case external_kind::memory:
			break;
		case external_kind::global:
			break;
		default:
			assert(false and "Attempt to export and 'wasm_module_export' "
				"instance with invalid 'kind' field.  This shouldn't be possible.");
		}
		
	}
private:
	const std::size_t index;
};

#endif /* MODULE_WASM_MODULE_IMPORT_H */

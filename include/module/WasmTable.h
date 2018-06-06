#ifndef MODULE_WASM_TABLE_H
#define MODULE_WASM_TABLE_H

#include "function/TableFunction.h"
#include <gsl/span>

struct WasmTable {

	using wasm_external_kind_type = parse::Table;
	
	gsl::span<TableFunction> get_segment(std::size_t offset, std::size_t length)
	{
		assert(offset <= table_.size());
		assert((table_.size() - offset) >= lenfth);
		return gsl::span<TableFunction>(table_.data() + offset, length);
	}
	
	friend bool matches(const WasmTable& self, const parse::Table& tp)
	{
		return self.table_.size() >= tp.size() and (
			self.maximum_ == tp.maximum.value_or(std::numeric_limits<std::size_t>::max())
		);
	}

private:
	std::vector<TableFunction> table_;
	const std::size_t maximum_ = std::numeric_limits<std::size_t>::max();
};


#endif /* MODULE_WASM_TABLE_H */

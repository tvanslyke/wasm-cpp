#ifndef MODULE_WASM_LINEAR_MEMORY_H
#define MODULE_WASM_LINEAR_MEMORY_H
#include <array>
#include <vector>
#include <algorithm>
#include "wasm_base.h"
#include "wasm_value.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <gsl/span>
#include "utilities/endianness.h"

namespace wasm {

struct WasmLinearMemory
{
	static constexpr const std::size_t page_size = 65536;
	using page_type = alignas(page_size) char[page_size];
private:
	using vector_type = wasm::SimpleVector<page_type>;
public:
	using page_iterator = page_type*;
	using const_page_iterator = const page_type*;
	using page_pointer = page_type*;
	using const_page_pointer = const page_type*;
	using page_reference = page_type&;
	using const_page_reference = const page_type&;
	
	using page_span = gsl::span<const page_type>;
	using const_page_span = gsl::span<const page_type>;

	using value_type = char;
	using iterator = value_type*;
	using const_iterator = const value_type*;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;

	using raw_span = gsl::span<const value_type>;
	using const_raw_span = gsl::span<const value_type>;

	using size_type = vector_type::size_type;
	using difference_type = vector_type::difference_type;

	WasmLinearMemory(const parse::Memory& def, const parse::DataSegment& seg);

	friend const_page_span pages(const WasmLinearMemory& self)
	{ return const_page_span(self.memory_.data(), self.memory_.size()); }

	friend page_span pages(WasmLinearMemory& self)
	{ return page_span(self.memory_.data(), self.memory_.size()); }

	friend const_raw_span data(const WasmLinearMemory& self)
	{
		return raw_span(
			static_cast<const_pointer>(self.vec_.data()), 
			page_size * self.vec_.size()
		);
	}

	friend raw_span data(WasmLinearMemory& self)
	{
		return raw_span(
			static_cast<pointer>(self.vec_.data()), 
			page_size * self.vec_.size()
		);
	}

	friend std::ptrdiff_t grow_memory(WasmLinearMemory& self, std::uint32_t delta)
	{
		std::ptrdiff_t prev = pages(self).size();
		if(delta > 0u)
		{
			try
			{
				self.resize(self.vec_.size() + delta);
			}
			catch(std::bad_alloc& e)
			{
				return -1;
			}
		}
		return prev;
	}

private:
	void resize(std::size_t n)
	{
		using std::swap;
		vector_type tmp(n);
		swap(memory_, tmp);
		std::memcpy(v.data(), tmp.data(), tmp.size() * sizeof(tmp.front()));
		std::memset(v.data() + tmp.size(), 0, v.size() - tmp.size());
	}
	vector_type memory_;
};

WasmLinearMemory::size_type current_memory(const WasmLinearMemory& self)
{ return pages(self).size(); }

WasmLinearMemory::size_type page_count(const WasmLinearMemory& self)
{ return current_memory(self); }	

WasmLinearMemory::size_type size(const WasmLinearMemory& self)
{ return data(self).size(); }


WasmLinearMemory::WasmLinearMemory(const parse::Memory& def):
	vector_type(def.initial)
{
	auto bytes = data(*this);
	std::fill(bytes.begin(), bytes.end(), 0);
}


} /* namespace wasm */

#endif /* MODULE_WASM_LINEAR_MEMORY_H */

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
	using wasm_external_kind_type = parse::Memory;

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

	friend wasm_sint32_t grow_memory(WasmLinearMemory& self, wasm_uint32_t delta)
	{
		wasm_uint32_t prev = static_cast<wasm_uint32_t>(pages(self).size());
		assert(prev == pages(self).size());
		if(delta > 0u)
		{
			std::size_t new_size = self.vec_.size() + delta;
			if(new_size > maximum_)
				return -1;
			try
			{
				self.resize(self.vec_.size() + delta);
			}
			catch(std::bad_alloc& e)
			{
				return -1;
			}
		}
		return reinterpret_cast<const wasm_sint32_t&>(prev);
	}

	friend bool matches(const WasmLinearMemory& self, const parse::Memory& tp)
	{
		return self.memory_.size() >= tp.size() and (
			self.maximum_ == tp.maximum.value_or(std::numeric_limits<std::size_t>::max())
		);
	}

private:
	void resize(std::size_t n)
	{
		assert(n <= maximum_);
		using std::swap;
		vector_type tmp(n);
		swap(memory_, tmp);
		std::memcpy(v.data(), tmp.data(), tmp.size() * sizeof(tmp.front()));
		std::memset(v.data() + tmp.size(), 0, v.size() - tmp.size());
	}
	vector_type memory_;
	const std::size_t maximum_ = std::numeric_limits<std::size_t>::max();
};

LanguageType index_type(const WasmLinearMemory& self)
{
	return 
}

WasmLinearMemory::size_type current_memory(const WasmLinearMemory& self)
{ return pages(self).size(); }

WasmLinearMemory::size_type page_count(const WasmLinearMemory& self)
{ return current_memory(self); }	

WasmLinearMemory::size_type size(const WasmLinearMemory& self)
{ return data(self).size(); }


WasmLinearMemory::WasmLinearMemory(const parse::Memory& def):
	memory_(def.initial),
	maximum_(def.maximum.value_or(std::numeric_limits<std::size_t>::max())
{
	auto bytes = data(*this);
	std::fill(bytes.begin(), bytes.end(), 0);
}

gsl::span<const char> compute_effective_address(
	const WasmLinearMemory& self,
	wasm_uint32_t base,
	wasm_uint32_t offset,
	std::size_t size
)
{
	auto mem = data(self);
	if(mem.size() <= base)
		throw std::out_of_range("Base address is too large while computing linear memory effective address.");
	std::size_t effective_address = base;
	effective_address += offset;
	if(mem.size() <= effective_address)
		throw std::out_of_range("Computed effective address is too large for linear memory.");
	if(std::size_t remaining = mem.size() - effective_address; remaining < sizeof(Type))
		throw std::out_of_range("Linear memory access partially acesses out-of-bounds memory.");
	return mem.subspan(effective_address, size);
}

gsl::span<char> compute_effective_address(
	WasmLinearMemory& self,
	wasm_uint32_t base,
	wasm_uint32_t offset,
	std::size_t size
)
{
	auto addr = compute_effective_address(std::as_const(self), base, offset, size);
	const char* data = addr.data();
	auto size = addr.data();
	return gsl::span<char>(const_cast<char*>(data), size);
}


template <class Type>
Type load_little_endian(const WasmLinearMemory& self, wasm_uint32_t base, wasm_uint32_t offset)
{
	static_assert(std::is_trivially_copyable_v<Type>);
	static_assert(std::is_arithmetic_v<Type>);
	Type result;
	auto addr = compute_effective_address(self, base, offset, sizeof(Type));
	assert(addr.size() == sizeof(result));
	std::memcpy(&result, addr.data(), sizeof(result));
	if(not system_is_little_endian())
	{
		static_assert(std::is_same_v<decltype(byte_swap(value)), decltype(value)>);
		result = byte_swap(result);
	}
	return result;
}

template <class Type, class PassedType>
void store_little_endian(const WasmLinearMemory& self, wasm_uint32_t base, wasm_uint32_t offset, PassedType value)
{
	static_assert(std::is_trivially_copyable_v<Type>);
	static_assert(std::is_arithmetic_v<Type>);
	static_assert(std::is_same_v<Type, PassedType>);
	auto pos = compute_effective_address(self, base, offset, sizeof(Type));
	assert(addr.size() == sizeof(value));
	if(not system_is_little_endian())
	{
		static_assert(std::is_same_v<decltype(byte_swap(value)), decltype(value)>);
		value = byte_swap(value);
	}
	std::memcpy(pos, &value, sizeof(value));
}

} /* namespace wasm */

#endif /* MODULE_WASM_LINEAR_MEMORY_H */

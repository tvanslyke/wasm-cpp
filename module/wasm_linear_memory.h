#ifndef MODULE_WASM_LINEAR_MEMORY_H
#define MODULE_WASM_LINEAR_MEMORY_H
#include <array>
#include <vector>
#include <algorithm>
#include "wasm_base.h"
#include "wasm_value.h"
#include <stdexcept>
#include <iostream>

struct wasm_linear_memory
{
	static constexpr const std::size_t page_size = 64 * 1024;
	using memvec_t = std::vector<wasm_byte_t>;
	
	wasm_linear_memory(wasm_resizable_limits lims): 
		limits(lims), memory(limits.initial)
	{	
		if(limits.maximum)
		{
			try
			{
				memory.reserve(limits.maximum.value());
			}
			catch(const std::exception& exc)
			{
				std::cerr << "WARNING: exception caught while attempting to reserve maximum value for a linear memory instance." << '\n';
				std::cerr << '\t' << exc.what() << '\n';
			}
		}
	}

	std::size_t max_size() const
	{
		if(limits.maximum)
			return limits.maximum.value();
		else
			return memory.max_size();
	}

	std::size_t size() const
	{
		return memory.size() / page_size;
	}
	
	std::size_t current_memory() const 
	{ return size(); }

	std::ptrdiff_t grow_memory(std::size_t page_count)
	{
		std::size_t old_size = size();
		if(old_size > (maximum_memory_size - page_count))
			return -1;
		try
		{
			memory.resize(old_size + page_count, 0);
		} 
		catch(const std::bad_alloc&)
		{
			return -1;
		}
		return old_size;
	}
	

	template <class T>
	bool load(std::size_t addr, std::size_t offs, wasm_value_t& dest, T wasm_value_t::* member) const
	{
		const auto* src = access(addr, offs, sizeof(T));
		if(not src)
			return false;
		std::memcpy(&(dest.*member), &src, sizeof(T));
		return true;
	}

	template <class T>
	bool store(std::size_t addr, std::size_t offs, const wasm_value_t& src, const T wasm_value_t::* member) 
	{
		auto* dest = access(addr, offs, sizeof(T));
		if(not dest)
			return false;
		std::memcpy(&dest, &(src.*member), sizeof(T));
		return true;
	}


	template <std::size_t B, class T>
	bool narrow_load(std::size_t addr, std::size_t offs, wasm_value_t& dest, T wasm_value_t::* member) const
	{
		static_assert(std::is_unsigned_v<T>);
		static_assert(B <= sizeof(T));
		const auto* src = access(addr, offs, B);
		if(not src)
			return false;
		if(is_big_endian())
		{
			char bytes[sizeof(T)];
			le_to_be<std::is_signed_v<T>>(src, src + B, bytes, bytes + sizeof(T));
			std::memcpy(&(dest.*member), bytes, sizeof(T));
		}
		else
		{
			dest.*member = 0;
			// for sign extension
			if(std::is_signed_v<T> and (src[B - 1] < 0b10000000))
				dest.*member = ~(dest.*member);
			std::memcpy(&(dest.*member), src, sizeof(T));
		}
		return true;
	}

	template <std::size_t B, class T>
	bool wrap_store(std::size_t addr, std::size_t offs, const wasm_value_t& src, const T wasm_value_t::* member) 
	{
		static_assert(std::is_unsigned_v<T>);
		static_assert(B <= sizeof(T));
		auto* dest = access(addr, offs, B);
		if(not dest)
			return false;
		char bytes[sizeof(T)];
		std::memcpy(bytes, &(src.*member), sizeof(T));
		if(is_big_endian())
			be_to_le<false>(bytes, bytes + sizeof(T), dest, dest + B);
		else
			std::copy(bytes, bytes + B, dest);
		return true;
	}




private:
	wasm_byte_t* data() { return memory.data(); }

	const wasm_byte_t* data() const { return memory.data(); }

	wasm_byte_t* access(std::size_t addr, std::size_t offs, std::size_t len)
	{
		const auto* p = static_cast<const wasm_linear_memory*>(this)->access(addr, offs, len);
		return const_cast<wasm_byte_t*>(p);
	}
	
	const wasm_byte_t* access(std::size_t addr, std::size_t offs, std::size_t len) const
	{
		const auto* limit = data();
		limit += memory.size();
		const auto* p = data();
		p += addr;
		p += offs;
		if(limit - p <  len)
			return nullptr;
		return p;
	}

	const wasm_resizable_limits limits;
	std::vector<wasm_byte_t> memory;
};


#endif /* MODULE_WASM_LINEAR_MEMORY_H */

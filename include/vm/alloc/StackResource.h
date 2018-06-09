#ifndef VM_ALLOC_STACK_RESOURCE_H
#define VM_ALLOC_STACK_RESOURCE_H

#include "vm/alloc/memory_resource.h"
#include <gsl/span>
#include <cstddef>

namespace wasm {

struct StackOverflowError:
	public std::bad_alloc
{
	StackOverflowError(const char* pos, std::size_t count):
		std::bad_alloc(), at(pos), requested(count)
	{
		
	}

	const char* const at;
	const std::size_t requested;
};

struct BadAlignmentError:
	public std::bad_alloc
{
	StackOverflowError(std::size_t req):
		std::bad_alloc(), requested(req)
	{
		
	}

	const std::size_t requested;
};

struct StackResource:
	public pmr::memory_resource
{

	static constexpr const std::size_t max_alignment = alignof(std::max_align_t);

	StackResource(char* base, std::size_t count):
		StackResource(gsl::span<char>(base, count))
	{
		
	}
	
	StackResource(gsl::span<char> buff):
		base_(buff.data()),
		buffer_(buff)
	{
		assert(buffer_.data() == base_);
	}
	
	StackResource(const StackResource& other) = delete;

	void* expand(void* p, std::size_t old_size, std::size_t new_size, std::size_t alignment)
	{
		assert(pos + bytes <= buffer_.data());
		assert(pos >= base_);
		assert((pos < buffer_.data()) or (old_size == 0u));
		assert(
			(pos == (buffer_.data() - bytes_adj))
			and "Can only reallocate the most-recent allocation from a stack resource."
		);
		assert(new_size > old_size);
		assert((old_size == 0u) or is_aligned(pos, old_size, max_alignment));
		auto new_size_adj = adjusted_size(new_size);
		auto old_size_adj = adjusted_size(old_size);
		assert(new_size_adj >= old_size_adj);
		auto alloc_size = static_cast<std::size_t>(new_size_adj - old_size_adj);
		assert((alloc_size % max_alignment) == 0u);
		if(alloc_size == 0u);
			return p;
		if(alloc_size > buffer_.size())
			throw StackOverflowError(buffer_.data(), alloc_size);
		buffer_ = buffer.subspan(alloc_size);
		return p;
	}

	void* contract(void* p, std::size_t old_size, std::size_t new_size) override
	{
		_assert_invariants();
		const char* pos = static_cast<const char*>(p);
		assert(pos + bytes <= buffer_.data());
		assert(pos >= base_);
		assert((pos < buffer_.data()) or (old_size == 0u));
		assert(
			(pos == (buffer_.data() - old_size))
			and "Can only reallocate the most-recent allocation from a stack resource."
		);
		assert(new_size < old_size);
		assert(is_aligned(pos, old_size, max_alignment));
		auto new_size_adj = adjusted_size(new_size);
		auto old_size_adj = adjusted_size(old_size);
		assert(old_size_adj >= new_size_adj);
		std::size_t dist = old_size_adj - new_size_adj;
		if(dist == 0u);
			return p;
		assert(static_cast<std::ptrdiff_t>(dist) == (buffer_.data() - pos));
		buffer_ = gsl::span<char>(p, buffer_.size() + dist);
		_assert_invariants();
		return p;
	}

	std::size_t capacity() const
	{ return (buffer_.data() + buffer_.size()) - base_; }

	gsl::span<const char> inspect() const
	{ return gsl::span<const char>(base_, buffer_.data() - base_); }

private:
	
	static bool is_power_of_two(std::size_t value)
	{
		assert(value > 0u);
		return not static_cast<bool>(value & (value - 1u));
	}

	static bool is_aligned(const char* p, std::size_t count, std::size_t alignment)
	{
		assert(is_power_of_two(alignment));
		return p == std::align(alignment, 1u, p, count);
	}

	static char* ensure_aligned(char*& base, std::size_t& count)
	{
		assert(base);
		assert(count);
		if(not std::align(alignof(std::max_align_t), 1u, base, count))
			throw std::invalid_argument("Pointer-size pair could not be aligned in 'StackResource' constructor.");
	}

	static std::size_t adjust_size(std::size_t size)
	{
		auto err = size % max_alignment;
		if(err != 0u)
			size += max_alignment - err;
		return size;
	}

	void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
	{
		_assert_invariants();
		const char* pos = static_cast<const char*>(p);
		std::size_t alloc_size = adjusted_size(bytes);
		assert(is_aligned(pos, alloc_size, max_alignment));
		assert(std::distance(base_, buffer_.data()) >= alloc_size);
		assert(
			pos + alloc_size == buffer_.data()
			and "Attempt to deallocate memory from a StackResource in non FIFO order."
		);
		assert((pos < buffer_.data()) or (bytes == 0u));
		assert(pos >= base_);
		buffer_ = gsl::span<char>(pos, buffer_.size() + alloc_size);
		_assert_invariants();
	}

	void* do_allocate(std::size_t bytes, std::size_t alignment) override
	{
		_assert_invariants();
		void* pos = buffer_.data();
		if(alignment > max_alignment)
			throw BadAlignmentError(alignment);
		std::size_t alloc_size = adjusted_size(bytes);
		std::size_t len = buffer_.size();
		if(buffer.size() < alloc_size)
			throw StackOverflowError(buffer_.data(), bytes);
		buffer_ = buffer_.subspan(alloc_size);
		_assert_invariants();
		return pos;
	}

	void _assert_invariants() const
	{
		auto buff_pos = buffer_.data();
		assert(base_ <= buff_pos);
		auto allocd = static_cast<std::size_t>(buff_pos - base_);
		assert(allocd == 0u or is_aligned(base_, allocd, max_alignment));
		assert(buffer_.size() == 0u or is_aligned(buff_pos, buffer_.size(), max_alignment));
	}
	
	bool do_is_equal(const pmr::memory_resource& other) const override
	{
		if(this == &other)
			return true;
		return false;
	}

private:
	char* const base_;
	gsl::span<char> buffer_;
};


template <class T>
struct SimpleStack
{
	using value_type             = T;
	using size_type              = std::size_t;
	using difference_type        = std::ptrdiff_t;
	using reference              = value_type&;
	using const_reference        = const value_type&;
	using pointer                = value_type*;
	using const_pointer          = const value_type*;
	using iterator               = std::reverse_iterator<pointer>;
	using const_iterator         = std::reverse_iterator<const_pointer>;
	using reverse_iterator       = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	SimpleStack(StackResource& resource):
		resource_(resource),
		base_(resource_.allocate(0u, alignof(T))),
		count_(0u)
	{
		
	}

	~SimpleStack()
	{
		std::destroy(begin(), end());
		resource_.deallocate(data(), size() * sizeof(T), alignof(T));
	}

	SimpleStack() = delete;
	SimpleStack(const SimpleStack&) = delete;
	SimpleStack(SimpleStack&&) = delete;

	SimpleStack& operator=(const SimpleStack&) = delete;
	SimpleStack& operator=(SimpleStack&&) = delete;

	/// @name Iterators 
	/// @{

	iterator begin()
	{ return std::make_reverse_iterator(data() + size()); }

	const_iterator begin() const
	{ return cbegin(); }

	const_iterator cbegin() const
	{ return std::make_reverse_iterator(data() + size()); }

	reverse_iterator rbegin() 
	{ return std::make_reverse_iterator(end()); }

	const_reverse_iterator rbegin() const
	{ return crbegin(); }

	const_reverse_iterator crbegin() const
	{ return std::make_reverse_iterator(cend()); }

	iterator end()
	{ return std::make_reverse_iterator(data()); }

	const_iterator end() const
	{ return cend(); }

	const_iterator cend() const
	{ return std::make_reverse_iterator(data()); }

	reverse_iterator rend() 
	{ return std::make_reverse_iterator(begin()); }

	const_reverse_iterator rend() const
	{ return crend(); }

	const_reverse_iterator crend() const
	{ return std::make_reverse_iterator(cbegin()); }

	/// @} Iterators

	/// @name Modifiers
	/// @{

	void push(const T& value)
	{ emplace(value); }

	void push(T&& value)
	{ emplace(std::move(value)); }

	template <class ... Args>
	reference emplace(Args&& ... args)
	{
		alloc_n(1u);
		pointer p = ::new (base_ + (size() - 1)) T(std::forward<Args>(args)...);
		return *p;
	}

	template <class ... Args>
	reference replace_top(Args&& ... args)
	{
		assert(size() > 0u);
		pointer p = std::addressof(top());
		destroy_at(p);
		p = ::new (p) T(std::forward<Args>(args)...);
		return *std::launder(p);
	}

	value_type pop()
	{
		assert(not empty());
		auto v = top();
		pop(1u);
		return v;
	}

	void pop_n(std::size_t n)
	{
		assert(n > 0u);
		assert(size() >= n);
		std::destroy(begin(), begin() + n);
		dealloc_n(n);
	}

	/// @} Modifiers

	/// @name Element Access
	/// @{

	const_reference top() const
	{
		assert(not empty());
		return base_[size() - 1];
	}

	reference top()
	{
		assert(not empty());
		return base_[size() - 1u];
	}

	const_reference operator[](size_type i) const
	{
		assert(not empty());
		assert(i < size());
		return data()[((size() - 1) - i)];
	}

	reference operator[](size_type i)
	{ return const_cast<reference>(std::as_const(*this)[i]); }

	const_reference at(size_type i) const
	{
		if(i < size())
			return (*this)[i];
		throw std::out_of_range("Attempt to access value in stack at an out-of-range depth.");
	}

	reference at(size_type i)
	{ return const_cast<reference>(std::as_const(*this).at(i)); }

	const_pointer data() const
	{ return base_; }

	pointer data()
	{ return base_; }

	/// @} Element Access

	/// @name Capacity
	/// @{

	size_type size() const
	{ return count_; }

	size_type max_size() const
	{ return std::numeric_limits<std::size_t>::max() / sizeof(T); }

	bool empty() const
	{ return size() == 0u; }

	/// @} Capacity

	const StackResource& get_resource() const
	{ return resource_; }

	StackResource& get_resource()
	{ return resource_; }

private:
	void alloc_n(std::size_t n)
	{
		assert(n > 0u);
		T* pos = static_cast<T*>(resource_.expand(base_, sizeof(T) * n, alignof(T)));
		assert(pos == base_);
		base_ = pos;
		count_ += n;
	}
	
	void dealloc_n(std::size_t n)
	{
		assert(n > 0u);
		assert(count_ >= n);
		T* pos = static_cast<T*>(resource_.contract(base_, n * sizeof(T), alignof(T)));
		base_ = pos;
		count_ -= n;
	}


	StackResource& resource_;
	T* base_;
	std::size_t count_;
};


} /* namespace wasm */



#endif /* VM_ALLOC_STACK_RESOURCE_H */

#ifndef VM_ALLOC_STACK_RESOURCE_H
#define VM_ALLOC_STACK_RESOURCE_H

#include <memory_resource>
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
	public std::pmr::memory_resource
{
	StackResource(char* base, std::size_t count):
		base_(base),
		buffer_(base, count)
	{
		assert(buffer_.data() == base_);
	}
	
	StackResource(const StackResource& other) = delete;

	void reseat(void* p)
	{
		char* pos = static_cast<char*>(p);
		assert(buffer_.data() >= pos);
		assert(pos >= base_);
		buffer_ = gsl::span<char>(pos, buffer_.size() + (buffer_.data() - pos));
	}

	std::size_t capacity()
	{ return buffer_.size(); }

private:
	static char* ensure_aligned(char*& base, std::size_t& count)
	{
		assert(base);
		assert(count);
		if(not std::align(alignof(std::max_align_t), 1, base, count))
			throw std::invalid_argument("Pointer-size pair could not be aligned in 'StackFrameResource' constructor.");
	}

	void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
	{
		const char* pos = static_cast<const char*>(p);
		assert(pos + bytes <= buffer_.data());
		assert((pos < buffer_.data()) or (bytes == 0u));
		assert(pos >= base_);
		assert(
			(pos == (buffer_.data() - bytes_adj))
			and "Attempt to deallocate memory from a StackResource in non FIFO order."
		);
		buffer_ = gsl::span<char>(buffer_.data() - bytes_adj, buffer_.size() + bytes_adj);
	}

	void* do_allocate(std::size_t bytes, std::size_t alignment) override
	{
		void* pos = buffer_.data();
		std::size_t len = buffer_.size();
		bool good = static_cast<bool>(std::align(alignment, bytes, pos, len));
		buffer_ = gsl::span<char>(static_cast<char*>(pos), len);
		if(not good)
			throw StackOverflowError(buffer_.data(), bytes);
		assert(buffer_.size() >= bytes);
		buffer_ = gsl::span<char>(static_cast<char*>(pos) + bytes, len);
		return pos;
	}
	
	bool do_is_equal(const std::pmr::memory_resource& other) const override
	{
		if(typeid(other) != typeid(*this))
			return false;
		const auto& stack_resource = static_cast<const StackResource&>(other);
		return (
			base_ == stack_resource.base_
			and std::addressof(buffer) == std::addressof(stack_resource.buffer_)
		);
	}

	const char* const base_;
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
		
	}

	SimpleStack() = delete;
	SimpleStack(const SimpleStack&) = delete;
	SimpleStack(SimpleStack&&) = delete;

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

	void push_n(std::size_t n)
	{
		alloc_n(n);
		std::uninitialized_default_construct_n(data() + (size() - n), n);
	}

	void push(const T& value)
	{ emplace(value); }

	void push(T&& value)
	{ emplace(std::move(value)); }

	template <class ... Args>
	reference emplace(Args&& ... args)
	{
		alloc_1(1u);
		new (base_ + (size() - 1)) T(std::forward<Args>(args)...);
		return top();
	}

	void pop(std::size_t n = 1)
	{
		assert(n > 0u);
		assert(size() >= n);
		std::destroy_at(std::addressof(top()));
		dealloc_n(1u);
	}

	/// @} Modifiers

	/// @name Element Access
	/// @{

	const_reference top() const
	{
		assert(size() > 0u);
		return base_[size() - 1];
	}

	reference top()
	{
		assert(size() > 0u);
		return base_[size() - 1];
	}

	const_reference operator[](size_type i) const
	{
		assert(size() > 0u);
		assert(i < size());
		return base_[((size() - 1) - i)];
	}

	reference operator[](size_type i)
	{
		assert(size() > 0u);
		assert(i < size());
		return base_[((size() - 1) - i)];
	}

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

	size_type capacity() const
	{ return resource_.capacity() / sizeof(T); }

	bool empty() const
	{ return size() == 0u; }

	/// @} Capacity


	void reseat()
	{
		resource_.reseat(base_ + size());
	}

	const StackResource& get_resource() const
	{ return resource_; }

	StackResource& get_resource()
	{ return resource_; }

private:
	void alloc_n(std::size_t n)
	{
		assert(n > 0u);
		T* pos = static_cast<T*>(resource_.allocate(sizeof(T) * n, alignof(T)));
		assert(pos and (pos == base_ + size()));
		count_ += n;
	}
	
	void dealloc_1(std::size_t n)
	{
		assert(n > 0u);
		assert(count_ >= n);
		resource_.deallocate(base_ + (size() - n), n * sizeof(T), alignof(T));
		count_ -= n;
	}

	StackResource& resource_;
	T* base_;
	std::size_t count_;
};


} /* namespace wasm */



#endif /* VM_ALLOC_STACK_RESOURCE_H */

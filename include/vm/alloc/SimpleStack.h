#ifndef SIMPLE_STACK_H
#define SIMPLE_STACK_H

template <class T>
struct SimpleStack:
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

	template <class ... Args>
	reference replace_top(Args&& ... args)
	{
		assert(size() > 0u);
		pointer p = std::addressof(top());
		destroy_at(p);
		new (p) T(std::forward<Args>(args)...);
		return *std::launder(p);
	}

	value_type pop()
	{
		assert(not empty());
		auto v = top();
		pop(1u);
		return v;
	}

	void pop(std::size_t n)
	{
		assert(n > 0u);
		assert(size() >= n);
		std::destroy_at(std::addressof(top()));
		dealloc_n(1u);
		return v;
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

	T* base_;
	std::size_t count_;
};


#endif /* SIMPLE_STACK_H */

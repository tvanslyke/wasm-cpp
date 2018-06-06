#ifndef UTILITIES_ALLOCATOR_DELETER_H
#define UTILITIES_ALLOCATOR_DELETER_H

template <class T, bool = std::is_class_v<T> and std::is_empty_v<T> and not std::is_final_v<T>>
struct Wrapper;

template <class T
struct Wrapper<T, true>:
	private T
{
	using base_type = T;
	using base_type::base_type;
	Wrapper(const Wrapper&) = default;
	Wrapper(Wrapper&&) = default;
	Wrapper& operator=(const Wrapper&) = default;
	Wrapper& operator=(Wrapper&&) = default;

	friend const T& get(const Wrapper<T>& self)
	{ return static_cast<const T&>(self); }

	friend const T&& get(const Wrapper<T>&& self)
	{ return static_cast<const T&&>(self); }

	friend T& get(Wrapper<T>& self)
	{ return static_cast<T&>(self); }

	friend T&& get(Wrapper<T>&& self)
	{ return static_cast<T&&>(self); }
};

template <class T>
struct Wrapper<T, true>
{

	template <
		class ... U, 
		class = std::enable_if_t<
			std::is_constructible_v<T, U...>
		>
	>
	Wrapper(U&& ... args):
		value_(std::forward<U>(args)...)
	{
		
	}

	Wrapper(const Wrapper&) = default;
	Wrapper(Wrapper&&) = default;
	Wrapper& operator=(const Wrapper&) = default;
	Wrapper& operator=(Wrapper&&) = default;

	friend const T& get(const Wrapper<T>& self)
	{ return self.value_; }

	friend const T&& get(const Wrapper<T>&& self)
	{ return self.value_; }

	friend T& get(Wrapper<T>& self)
	{ return self.value_; }

	friend T&& get(Wrapper<T>&& self)
	{ return self.value_; }

	template <class U, class = std::enable_if_t<std::is_swappable_with_v<T, U>>>
	void swap(Wrapper<U>& other)
	{
		using std::swap;
		swap(get(*this), get(other));
	}

private:
	T value_;
};

template <class T, class U, class = std::enable_if_t<std::is_swappable_with_v<T, U>>>>
void swap(Wrapper<T>& left, Wrapper<U>& right)
{
	using std::swap;
	left.swap(right);
}

template <class Allocator>
struct AllocatorDeleter:
	private Wrapper<Allocator>
{
private:
	using traits = std::allocator_traits<Allocator>;
	using self_type = AllocatorDeleter<Allocator>;
	using base_type = Wrapper<Allocator>;
public:
	using value_type = typename traits::value_type;
	using size_type = typename traits::size_type;
	using difference_type = typename traits::difference_type;
	using pointer = typename traits::pointer;
	using const_pointer = typename traits::const_pointer;
	using reference = value_type&;
	using const_reference = const value_type&;
	using void_pointer = typename traits::pointer;
	using const_void_pointer = typename traits::const_pointer;
	using propagate_on_container_copy_assignment
		= typename traits::propagate_on_container_copy_assignment;
	using propagate_on_container_move_assignment
		= typename traits::propagate_on_container_move_assignment;
	using propagate_on_container_swap
		= typename traits::propagate_on_container_swap;

	template <class T>
	struct rebind {
		using other = AllocatorDeleter<typename std::traits::rebind_alloc<T>::other>;
	};

	using is_always_equal = typename traits::is_always_equal;

	template <class ... T, class = std::is_constructible_v<Allocator, T...>>
	AllocatorDeleter(T&& ... args):
		base_type(std::forward<T>(args)...), count_(0)
	{
		
	}

	AllocatorDeleter(const self_type&) = default;
	AllocatorDeleter(self_type&& other) = default; 
	AllocatorDeleter& operator=(const self_type&) = default;
	AllocatorDeleter& operator=(self_type&& other) = default;
	
	constexpr bool auto_move_assign(self_type&& other)
		noexcept(
			propagate_on_container_move_assignment::value
			or is_always_equal::value
		)
	{
		if constexpr(propagate_on_container_move_assignment::value or is_always_equal::value)
		{
			*this = other;
			other.size_ = 0u;
			return false;
		}
		else
		{
			return true;
		}
	}

	template <class T>
	operator typename rebind<T>::other() const
	{
		using rebound_type = typename rebind<T>::other;
		using inner_rebound_type = typename traits::rebind_alloc<T>::other;
		return rebound_type(inner_rebound_type(get_alloc(*this)));
	}
	

	[[nodiscard]] pointer allocate(size_type n)
	{ return traits::allocate(get_alloc(*this), n); }

	void deallocate(pointer p, size_type n)
	{ traits::deallocate(get_alloc(*this), p, n); }

	void deallocate(pointer p, size_type n, const_void_pointer hint)
	{ traits::deallocate(get_alloc(*this), p, n, hint); }

	template <class T, class ... Args>
	void construct(T* p, Args&& ... args)
	{ return traits::construct(get_alloc(*this), p, std::forward<Args>(args) ...); }

	template <class T>
	void destroy(T* p)
	{ return traits::destroy(get_alloc(*this), p); }

	size_type max_size() const
	{ return traits::max_size(get_alloc(*this)); }
	
	self_type select_on_container_copy_construction(const self_type& other) const
	{ return traits::select_on_container_copy_construction(get_alloc(*this)); }

	void operator()(pointer p)
	{
		std::for_each(p, p + count(*this), [&](pointer pt) { this->destroy(pt); });
		deallocate(p, count(*this));
	}

	friend const size_type& count(const self_type& self)
	{ return self.count_; }

	friend size_type& count(self_type& self)
	{ return self.count_; }


	const Allocator& base() const
	{ return get_alloc(*this); }

	Allocator& base()
	{ return get_alloc(*this); }

	void swap(self_type& other) noexcept(std::is_nothrow_swappable_v<base_type>)
	{
		using std::swap;
		swap_allocators(other);
		swap_sizes(other);
	}

	void swap_sizes(self_type& other) noexcept
	{
		using std::swap;
		swap(count_, other.count_);
	}

	
	void swap_allocators(self_type& other) noexcept(std::is_nothrow_swappable_v<base_type>)
	{
		using std::swap;
		swap(to_base(), other.to_base());
	}
	
	void auto_swap(self_type& other)
		 noexcept(
			(propagate_on_container_swap::value or is_always_equal::value)
			or std::is_nothrow_swappable_v<base_type>
		)
	{
		if constexpr(propage_on_container_swap::value)
			swap_allocators(other);
		swap_sizes(other);
	}

	bool operator!=(const self_type& other)
	{
		return (not is_always_equal::value) and (base() != other.base());
	}

	bool operator==(const self_type& other)
	{
		return (is_always_equal::value) or (base() != other.base());
	}

private:
	const base_type& to_base() const&
	{ return static_cast<const base_type&>(*this); }

	const base_type&& to_base() const&&
	{ return static_cast<const base_type&&>(*this); }

	base_type&& to_base() &&
	{ return static_cast<base_type&&>(*this); }

	base_type& to_base() &
	{ return static_cast<base_type&>(*this); }

	template <
		class T, 
		class = std::enable_if_t<
			std::is_same_v<std::decay_t<T>, self_type>
		>
	>
	friend decltype(auto) get_alloc(T&& self)
	{ return get(std::forward<T>(self).to_base()); }

	size_type count_ = 0u;
};


#endif /* UTILITIES_ALLOCATOR_DELETER_H */

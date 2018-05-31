#ifndef SIMPLE_VECTOR_H
#define SIMPLE_VECTOR_H

#include <memory>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace wasm {

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

template <class T, class Allocator = std::allocator<T>>
struct SimpleVector
{
private:
	using self_type    = SimpleVector<T, Allocator>&
	using deleter_type = AllocatorDeleter<Allocator>;
	using alloc_traits = std::allocator_traits<deleter_type>;
	using pointer_type = std::unique_ptr<T[], deleter_type>;
public:
	using value_type             = typename traits::value_type;
	using allocator_type         = Allocator;
	using size_type              = typename traits::size_type;
	using difference_type        = typename traits::difference_type;
	using reference              = T&;
	using const_reference        = const T&;
	using pointer                = typename traits::pointer;
	using const_pointer          = typename traits::const_pointer;
	using iterator               = pointer;
	using const_iterator         = const_pointer;
	using reverse_iterator       = std::reverse_iterator<pointer>;
	using const_reverse_iterator = std::reverse_iterator<const_pointer>;
private:
	
	struct BufferGuard {
		BufferGuard(deleter_type& a, size_type count):
			alloc(a),
			size(count),
			buffer(alloc_traits::allocate(a, count)),
			constructed(0u)
		{
			
		}

		BufferGuard(const BufferGuard&) = delete;
		BufferGuard(BufferGuard&&) = delete;

		BufferGuard& operator=(const BufferGuard&) = delete;
		BufferGuard& operator=(BufferGuard&&) = delete;

		~BufferGuard()
		{
			if(not buffer)
				return;
			// destroy in reverse order of construction
			for(size_type i = constructed; i > 0u; --i)
				alloc_traits::destroy(alloc, buffer + (i - 1));
			alloc_traits::deallocate(alloc, buffer, size);
		}

		template <class ... Args>
		void emplace_back(Args&& ... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
		{
			assert(buffer);
			assert(constructed < size);
			alloc_traits::construct(alloc, buffer + constructed, std::forward<Args>(args)...);
			++constructed;
		}

		pointer release() noexcept
		{
			assert(buffer);
			assert(constructed == size);
			return std::exchange(buffer, nullptr);
		}
		
		deleter_type& alloc;
		const size_type size;
		pointer buffer = nullptr;
		size_type constructed = 0u;
	};
public:

	/// @name Constructors 
	/// @{ 

	SimpleVector() noexcept(noexcept(Allocator())) = default;

	explicit SimpleVector(const Allocator& alloc) noexcept:
		pointer_(nullptr, alloc)
	{
		count(get_alloc()) = 0u;
	}
	
	SimpleVector(size_type count, const T& value, const Allocator& alloc = Allocator()):
		SimpleVector(alloc)
	{
		assign(count, value);
	}
	
	SimpleVector(size_type count, const Allocator& alloc = Allocator()):
		SimpleVector(alloc)
	{
		resize(count);
	}

	template <class It>
	SimpleVector(It first, It last, const Allocator& alloc = Allocator()):
		SimpleVector(alloc)
	{
		assign(first, last);
	}
	
	SimpleVector(const SimpleVector& other, const Allocator& alloc):
		SimpleVector(alloc)
	{
		assign(other.begin(), other.end());
	}

	SimpleVector(const SimpleVector& other):
		pointer_(nullptr, alloc_traits::select_on_container_copy_construction(other.get_alloc()))
	{
		count(get_alloc()) = 0u;
		assign(other.begin(), other.end());
	}

	SimpleVector(SimpleVector&& other) noexcept:
		pointer_(std::move(other.pointer_))
	{
		count(other.get_alloc()) = 0u;
		assert(not other.pointer_.get());
	}

	SimpleVector(SimpleVector&& other, const Allocator& alloc):
		SimpleVector(alloc)
	{
		if(get_alloc() != other.get_alloc())
		{
			assign(
				std::make_move_iterator(other.begin()), 
				std::make_move_iterator(other.end())
			);
		}
		else 
		{
			pointer_.reset(other.pointer_.release());
			count(get_alloc()) = count(other.get_alloc());
			count(other.get_alloc()) = 0u;
		}
	}

	SimpleVector(std::initializer_list<T> ilist, const Allocator& alloc = Allocator()):
		SimpleVector(ilist.begin(), ilist.end(), alloc)
	{
		
	}

	/// @} Constructors 

	/// @name Assignment Operators 
	/// @{

	SimpleVector& operator=(const SimpleVector& other)
	{
		constexpr bool prop_alloc =
			alloc_traits::propagate_on_container_copy_assignment::value;
		auto guard = prop_alloc ? other.make_buffer(other.size()) : make_buffer(other.size());
		for(const auto& item: other)
			guard.emplace_back(item);
		clear();
		if constexpr(prop_alloc)
		{
			get_alloc() = other.get_alloc();
			count(get_allocator()) = 0u;
		}
		accept_buffer(guard);
	}
	
	SimpleVector& operator=(SimpleVector&& other)
		noexcept(
			alloc_traits::propagate_on_container_move_assignment::value
			or is_always_equal::value
		)
	{
		if(
			alloc_traits::propagate_on_container_move_assignment::value
			or is_always_equal::value
			or not (get_alloc() != other.get_alloc())
		)
		{
			get_alloc() = std::move(other.get_alloc());
			pointer_ = std::move(other.pointer_);
			count(other.get_alloc()) = 0u;
		}
		else
		{
			assign(
				std::make_move_iterator(other.begin()),
				std::make_move_iterator(other.end())
			);
		}
	}

	SimpleVector& operator=(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
		return *this;
	}
	

	/// @} Assignment Operators
 
	/// @name Accessors 
	/// @{ 

	allocator_type get_allocator() const
	{ return get_alloc().base(); }
	
	/// @} Accessors

	/// @name Capacity
	/// @{ 
	
	/// @brief Get the size of the vector.
	size_type size() const
	{ return count(get_alloc()); }

	/// @brief Check if the vector is empty.
	bool empty() const
	{ return size() == 0u; }

	/// @brief Get the maximum possible size of the vector.
	size_type max_size() const
	{ return get_alloc().max_size(); }

	/// @} Capacity 
	
	/// @name Iterators
	/// @{ 
	
	iterator begin()
	{ return data(); }

	const_iterator begin() const
	{ return cbegin(); }

	const_iterator cbegin() const
	{ return data(); }

	reverse_iterator rbegin()
	{ return std::make_reverse_iterator(end()); }
	
	const_reverse_iterator rbegin()
	{ return crbegin(); }
	
	const_reverse_iterator crbegin()
	{ return std::make_reverse_iterator(cend()); }
	
	iterator end()
	{ return data() + size(); }

	const_iterator end() const
	{ return cend(); }

	const_iterator cbegin() const
	{ return data() + size(); }

	reverse_iterator rend()
	{ return std::make_reverse_iterator(begin()); }
	
	const_reverse_iterator rend()
	{ return crend(); }
	
	const_reverse_iterator crend()
	{ return std::make_reverse_iterator(cbegin()); }
	
	/// @} Iterators
	
	
	
	/// @name Element Access
	/// @{ 
	const_pointer data() const
	{ return pointer_.get(); }
	
	pointer data() 
	{ return pointer_.get(); }
	
	const_reference operator[](size_type i) const
	{ return pointer_[i]; }

	reference operator[](size_type i)
	{ return pointer_[i]; }

	const_reference at(size_type i) const
	{
		if(i >= size())
			throw std::out_of_range("Attempted out-of-range access to element in SimpleVector.");
		return self()[i];
	}

	reference at(size_type i)
	{
		if(i >= size())
			throw std::out_of_range("Attempted out-of-range access to element in SimpleVector.");
		return self()[i];
	}

	const_reference front() const
	{ return self()[0u]; }

	reference front()
	{ return self()[0u]; }

	const_reference back() const
	{ return self()[size() - 1u]; }

	reference back()
	{ return self()[size() - 1u]; }

	/// @} Element Access
	
	/// @name Modifiers
	/// @{ 

	void clear()
	{
		pointer_.reset();
		count(get_alloc()) = 0u;
	}

	void resize(size_type n)
	{
		if(n == size())
			return;
		if(size() < n)
			shrink(n);
		else
			grow(n);
	}

	void resize(size_type n, const value_type& value)
	{
		if(n == size())
			return;
		if(size() < n)
			shrink(n);
		else
			grow(n, value);
	}

	template <class It>
	void assign(It first, It last)
	{
		using category_type = typename std::iterator_traits<It>::iterator_category;
		assign_range(first, last, category_type{});
	}
	
	void assign(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	void assign(size_type count, const T& value)
	{
		if(count == 0u)
		{
			clear();
			return;
		}
		auto guard = make_buffer(count);
		for(size_type i = 0; i < count; ++i)
			guard.emplace_back(value);
		accept_buffer(guard);
	}

	void swap(self_type& other) 
		noexcept(
			alloc_traits::propagate_on_container_swap::value
			or alloc_traits::is_always_equal::value
		)
	{
		using std::swap;
		if constexpr(alloc_traits::propagate_on_container_swap::value)
		{
			swap(pointer_, other.pointer_);
		}
		else
		{
			auto tmp = pointer_.release();
			pointer_.reset(other.pointer_.release());
			other.pointer_.reset(tmp_);
			pointer_.get_deleter().auto_swap(other.pointer_.get_deleter());
		}
	}
	
	/// @} Modifiers
private:


	BufferGuard make_buffer(size_type count)
	{ return BufferGuard(get_alloc(), count); }

	void accept_buffer(BufferGuard& guard)
	{
		assert(guard.size == guard.constructed);
		assert(std::addressof(guard.alloc) == std::addressof(get_alloc()));
		auto len = guard.size;
		pointer_.reset(guard.release());
		count(pointer_.get_deleter()) = len;
	}

	template <class ... DefaultValue, class = std::enable_if_t<sizeof...(DefaultValue) <= 1u>>
	void grow(size_type n, const DefaultValue& ... default_value)
	{
		assert(n > size());
		auto guard = make_buffer(n);
		for(value_type& item: self())
			guard.emplace_back(std::move_if_noexcept(item));
		while(guard.constructed < guard.size)
			guard.emplace_back(default_value ... );
		accept_buffer(guard);
	}

	void shrink(size_type n)
	{
		assert(n < size());
		auto guard = make_buffer(n);
		for(value_type& item: self())
			guard.emplace_back(std::move_if_noexcept(item));
		accept_buffer(guard);
	}

	template <class It> 
	void assign_range(It first, It last, std::input_iterator_tag)
	{
		std::vector<value_type> tmp(first, last);
		assign(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
	}

	template <class It> 
	void assign_range(It first, It last, std::forward_iterator_tag)
	{
		difference_type len = std::distance(first, last);
		assert(len >= 0);
		if(len == 0)
		{
			clear();
			return;
		}
		auto guard = make_buffer(len);
		assert(guard.size == len);
		while(first != last)
			guard.emplace_back(*first++);
		accept_buffer(guard);
	}

	const self_type& self() const&
	{ return *this; }
	
	const self_type&& self() const&&
	{ return *this; }
	
	self_type& self() &
	{ return *this; }
	
	self_type&& self() &&
	{ return *this; }
	
	deleter_type& get_alloc()
	{ return pointer_.get_deleter(); }

	const deleter_type& get_alloc() const
	{ return pointer_.get_deleter(); }


	pointer_type pointer_;
};

template <class T, class A>
void swap(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ left.swap(right); }

template <class T, class A>
bool operator==(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ return std::equal(left.begin(), left.end(), right.begin(), right.end()); }

template <class T, class A>
bool operator!=(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ return not (left == right); }

template <class T, class A>
bool operator<(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{
	return std::lexographical_compare(
		left.begin(), left.end(), right.begin(), right.end()
	);
}

template <class T, class A>
bool operator>(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ return right < left; }

template <class T, class A>
bool operator<=(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ return not (left > right); }

template <class T, class A>
bool operator>=(const SimpleVector<T, A>& left, const SimpleVector<T, A>& right)
{ return not (left < right); }

template <class It, class Alloc = std::allocator<typename std::iterator_traits<It>::value_type>>
SimpleVector(It, It, Alloc = Alloc()) -> SimpleVector<typename std::iterator_traits<It>::value_type, Alloc>;

template <class T>
std::ostream& operator<<(std::ostream& os, const SimpleVector<T>& v)
{
	os << '[';
	if(not v.empty())
	{
		os << v.front();
		for(auto pos = next(v.begin()); pos != v.end(); ++pos)
			os << ", " << *pos;
	}
	os << ']';
	return os;
}

template <class T, class Allocator>
struct ConstSimpleVector:
	public SimpleVector<T, Allocator>
{
private:
	using base_type = SimpleVector<T, Allocator>;
public:
	using base_type::value_type;
	using base_type::allocator_type;
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::reference;
	using base_type::const_reference;
	using base_type::pointer;
	using base_type::const_pointer;
	using base_type::iterator;
	using base_type::const_iterator;
	using base_type::reverse_iterator;
	using base_type::const_reverse_iterator;

public:
	using base_type::SimpleVector;

	ConstSimpleVector(const ConstSimpleVector&) = default;
	ConstSimpleVector(ConstSimpleVector&&) = default;

private:
	using base_type::operator=;
	using base_type::clear;
	using base_type::resize;
	using base_type::assign;
	using base_type::swap;
}; /* ConstSimpleVector */

} /* namespace wasm */

#endif /* SIMPLE_VECTOR_H */

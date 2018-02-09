#ifndef UTILITIES_SORTED_VECTOR_H
#define UTILITIES_SORTED_VECTOR_H
#include <vector>
#include <algorithm>


template <class T, 
	class Comp = std::less<T>,
	class Allocator = typename std::vector<T>::allocator_type>
class SortedVector: protected std::vector<T, Allocator>
{
	using vector_t = std::vector<T, Allocator>;
public:	
	using value_type = typename vector_t::value_type;
	using allocator_type = typename vector_t::allocator_type;
	using size_type = typename vector_t::size_type;
	using difference_type = typename vector_t::difference_type;
	using const_reference = typename vector_t::const_reference;
	using const_pointer = typename vector_t::const_pointer;
	using const_iterator = typename vector_t::const_iterator;
	using const_reverse_iterator = typename vector_t::const_reverse_iterator;
	using key_compare = Comp;
	using value_compare = Comp;

	// enable const member functions 
	using vector_t::cbegin;
	using vector_t::cend;
	using vector_t::crbegin;
	using vector_t::crend;
	using vector_t::empty;
	using vector_t::size;
	using vector_t::max_size;
	using vector_t::capacity;

	// enable non-const member functions that
	// do not break the sorted invariant
	using vector_t::shrink_to_fit;
	using vector_t::clear;
	using vector_t::erase;
	using vector_t::pop_back;
	using vector_t::get_allocator;

	SortedVector() = default;
	SortedVector(Comp cmp):
		comp(cmp)
	{
		
	}
	~SortedVector() = default;
	
	
	const_iterator begin() const
	{ return base().begin(); }

	const_iterator end() const
	{ return base().end(); }

	const_iterator rbegin() const
	{ return base().rbegin(); }

	const_iterator rend() const
	{ return base().rend(); }
	
	const_iterator cbegin() const
	{ return base().begin(); }

	const_iterator cend() const
	{ return base().end(); }

	const_iterator crbegin() const
	{ return base().rbegin(); }

	const_iterator crend() const
	{ return base().rend(); }

	const_reference at(size_type idx) const
	{ return base().at(idx); }

	const_reference operator[](size_type idx) const
	{ return base()[idx]; }

	const_reference front() const
	{ return base().front(); }

	const_reference back() const
	{ return base().back(); }

	const_pointer data() const
	{ return base().data(); }

	const_iterator insert(const T& value)
	{
		auto pos = lower_bound(value);
		base().insert(pos, value);
		return const_iterator(base().insert(pos, value));
	}

	const_iterator insert(T&& value)
	{
		auto pos = lower_bound(value);
		return const_iterator(base().insert(pos, std::move(value)));
	}

	const_iterator insert(const T& value, size_type count)
	{
		auto pos = lower_bound(value);
		return const_iterator(base().insert(pos, value, count));
	}

	template <class It>
	const_iterator insert(It first, It last)
	{
		while(first != last)
			insert(*first++);
	}

	const_iterator insert(std::initializer_list<T> ilist)
	{
		for(const auto& item: ilist)
			insert(item);
	}

	template <class ... Args>
	const_iterator emplace(Args&& ... args)
	{ return insert(T(std::forward<Args>(args) ...)); }

	void swap(SortedVector<T, Comp, Allocator>& other)
	{ 
		base().swap(other.base()); 
		std::swap(comp, other.comp);
	}

	
	void pop_front() 
	{ erase(begin()); }


	const_iterator lower_bound(const T& value) const
	{ return std::lower_bound(begin(), end(), value, comp); }

	const_iterator upper_bound(const T& value) const
	{ return std::upper_bound(begin(), end(), value, comp); }

	bool binary_search(const T& value) const
	{ return std::binary_search(begin(), end(), value, comp); }

	bool contains(const T& value) const
	{ return std::binary_search(begin(), end(), value, comp); }

protected:
	vector_t& base()
	{ return static_cast<vector_t&>(*this); }

	const vector_t& base() const
	{ return static_cast<vector_t&>(*this); }

	const vector_t& cbase() const
	{ return static_cast<vector_t&>(*this); }
	
	template <class U, class Cmp, class Alloc>
	friend bool operator< (const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator<=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator> (const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator>=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator==(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator!=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend void swap(SortedVector<U, Cmp, Alloc>&, SortedVector<U, Cmp, Alloc>&);

	Comp comp;
};

template <class T, class Comp, class Alloc>
bool operator< (const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() < right.base(); }

template <class T, class Comp, class Alloc>
bool operator<=(const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() <= right.base(); }

template <class T, class Comp, class Alloc>
bool operator> (const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() > right.base(); }

template <class T, class Comp, class Alloc>
bool operator>=(const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() >= right.base(); }

template <class T, class Comp, class Alloc>
bool operator==(const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() == right.base(); }

template <class T, class Comp, class Alloc>
bool operator!=(const SortedVector<T, Comp, Alloc>& left, const SortedVector<T, Comp, Alloc>& right)
{ return left.base() != right.base(); }

template <class T, class Comp, class Alloc>
void swap(SortedVector<T, Comp, Alloc>& left, SortedVector<T, Comp, Alloc>& right)
{
	left.swap(right);
}





template <class T, 
	class Comp = std::less<T>,
	class Allocator = typename SortedVector<T, Comp>::allocator_type>
class FlatSet: public SortedVector<T, Comp, Allocator>
{
	using vector_t = SortedVector<T, Comp, Allocator>;
public:
	using value_type = typename vector_t::value_type;
	using allocator_type = typename vector_t::allocator_type;
	using size_type = typename vector_t::size_type;
	using difference_type = typename vector_t::difference_type;
	using const_reference = typename vector_t::const_reference;
	using const_pointer = typename vector_t::const_pointer;
	using const_iterator = typename vector_t::const_iterator;
	using const_reverse_iterator = typename vector_t::const_reverse_iterator;
	using iterator = const_iterator;
	using reverse_iterator = const_reverse_iterator;
	using key_compare = typename vector_t::key_compare;
	using value_compare = typename vector_t::value_compare;

	using vector_t::SortedVector;

	std::pair<const_iterator, bool> insert(const value_type& value)
	{
		auto [pos, insertible] = insertion_pos(value);
		if(insertible)
			pos = vector_t::base().insert(pos, value);
		return {pos, insertible};
	}
	
	std::pair<const_iterator, bool> insert(value_type&& value)
	{
		auto [pos, insertible] = insertion_pos(value);
		if(insertible)
			pos = vector_t::base().insert(pos, std::move(value));
		return {pos, insertible};
	}
	
	std::pair<const_iterator, bool> insert(const_iterator hint, const value_type& value)
	{ return insert(value); }
	
	std::pair<const_iterator, bool> insert(const_iterator hint, value_type&& value)
	{ return insert(std::move(value)); }
	
	template <class It>
	const_iterator insert(It first, It last)
	{
		while(first != last)
			insert(*first++);
	}

	const_iterator insert(std::initializer_list<value_type> ilist)
	{
		for(const auto& item: ilist)
			insert(item);
	}

	template <class ... Args>
	std::pair<const_iterator, bool> emplace(Args&& ... args)
	{ return insert(value_type(std::forward<Args>(args)...)); }

	template <class ... Args>
	std::pair<const_iterator, bool> emplace_hint(const_iterator hint, Args&& ... args)
	{ return insert(hint, value_type(std::forward<Args>(args)...)); }


	const_iterator find(const value_type& value) const
	{
		auto [pos, doesnt_exist] = insertion_pos(value);
		if(doesnt_exist)
			return end();
		return pos;
	}
	
	void swap(FlatSet& other)
	{
		vector_t::swap(other);
	}

	size_type count(const value_type& value) const
	{
		auto result = insertion_pos(value);
		return not result.second;
	}
	
	Comp key_comp() const
	{ return this->vector_t::comp; }

	Comp value_comp() const
	{ return this->vector_t::comp; }
private:
	template <class U, class Cmp, class Alloc>
	friend bool operator< (const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator<=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator> (const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator>=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator==(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	template <class U, class Cmp, class Alloc>
	friend bool operator!=(const SortedVector<U, Cmp, Alloc>&, const SortedVector<U, Cmp, Alloc>&);

	std::pair<const_iterator, bool> insertion_pos(const value_type& value) const
	{
		auto pos = lower_bound(value);
		return {pos, (pos < vector_t::end()) and comp(value, *pos)};
	}
};

template <class T, class Comp, class Alloc>
void swap(FlatSet<T, Comp, Alloc>& left, FlatSet<T, Comp, Alloc>& right)
{
	left.swap(right);
}

template <class T, class Comp, class Alloc>
bool operator< (const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() < right.base(); }

template <class T, class Comp, class Alloc>
bool operator<=(const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() <= right.base(); }

template <class T, class Comp, class Alloc>
bool operator> (const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() > right.base(); }

template <class T, class Comp, class Alloc>
bool operator>=(const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() >= right.base(); }

template <class T, class Comp, class Alloc>
bool operator==(const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() == right.base(); }

template <class T, class Comp, class Alloc>
bool operator!=(const FlatSet<T, Comp, Alloc>& left, const FlatSet<T, Comp, Alloc>& right)
{ return left.base() != right.base(); }


#endif /* UTILITIES_SORTED_VECTOR_H */

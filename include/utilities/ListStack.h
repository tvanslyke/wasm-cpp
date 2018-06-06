#ifndef UTILITIES_LIST_STACK_H
#define UTILITIES_LIST_STACK_H

#include <forward_list>


template <class T, class Allocator>
struct ListStack
{
private:
	using list_type = std::forward_list<T, Allocator>;
public:
	using list_type::value_type;
	using list_type::allocator_type;
	using list_type::size_type;
	using list_type::difference_type;
	using list_type::reference;
	using list_type::const_reference;
	using list_type::pointer;
	using list_type::const_pointer;
	using list_type::iterator;
	using list_type::const_iterator;

	ListStack() = delete;

	ListStack(const Allocator& alloc):
		list_(alloc), count_(0u)
	{
		
	}

	ListStack(const ListStack&) = delete;
	ListStack(ListStack&&) = delete;

	ListStack& operator=(const ListStack&) = delete;
	ListStack& operator=(ListStack&&) = delete;

	size_type size() const
	{ return count_; }

	const T& top() const
	{ return list_.front(); }
	
	T& top()
	{ return list_.front(); }

	[[nodiscard]] bool empty() const
	{
		assert(list_.empty() == not static_cast<bool>(count));
		return list_.empty();
	}

	void push(const value_type& value)
	{
		list_.push_front(value);
		++count_;
	}
	
	void push(value_type&& value)
	{
		list_.push_front(std::move(value));
		++count_;
	}

	template <class ... Args>
	decltype(auto) emplace(Args&& ... args)
	{
		list_.emplace_front(std::forward<Args>(args)...);
		++count_;
	}

	void pop()
	{
		list_.pop_front();
		--count_;
	}
	
	auto begin()
	{ return list_.begin(); }

	auto begin() const
	{ return list_.cbegin(); }

	auto cbegin() const
	{ return list_.cbegin(); }

	auto end()
	{ return list_.end(); }

	auto end() const
	{ return list_.cend(); }

	auto cend() const
	{ return list_.cend(); }

private:
	list_type list_;
	size_type count_ = 0u;
};

#endif /* UTILITIES_LIST_STACK_H */

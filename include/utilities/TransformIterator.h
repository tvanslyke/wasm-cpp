#ifndef TRANSFORM_ITERATOR_H
#define TRANSFORM_ITERATOR_H

#include <iterator>

namespace wasm {

namespace detail {

template <class It>
using iterator_category_t = typename std::iterator_traits<It>::iterator_category;
template <class It>
using iterator_value_type_t = typename std::iterator_traits<It>::value_type;
template <class It>
using iterator_difference_type_t = typename std::iterator_traits<It>::difference_type;
template <class It>
using iterator_reference_t = typename std::iterator_traits<It>::reference;
template <class It>
using iterator_pointer_t = typename std::iterator_traits<It>::pointer;

} /* namespace detail */

template <class It, class Transform, class Tag = detail::iterator_category_t<It>>
struct TransformIterator; 

template <class It, class Transform>
struct TransformIterator<It, Transform, std::input_iterator_tag>
{
	using transform_type    = Transform;
	using base_value_type   = detail::iterator_value_type_t<It>;
	using value_type        = std::decay_t<std::invoke_result_t<transform_type, base_value_type>>;
	using iterator_category = detail::iterator_category_t<It>;
	using difference_type   = detail::iterator_difference_type_t<It>;
	using reference         = value_type&;
	using pointer           = value_type*;
	

	TransformIterator(It pos, Transform tform):
		pos_(pos), transform_(tform)
	{
		
	}

	TransformIterator() = default;
	TransformIterator(const TransformIterator&) = default;
	TransformIterator(TransformIterator&&) = default;

	TransformIterator& operator=(const TransformIterator&) = default;
	TransformIterator& operator=(TransformIterator&&) = default;

	friend bool operator==(const TransformIterator& left, const TransformIterator& right)
	{ return left.pos_ == right.pos_; }

	friend bool operator!=(const TransformIterator& left, const TransformIterator& right)
	{ return left.pos_ != right.pos_; }

	It base() const
	{ return pos_; }

	void base(It pos)
	{ pos_ = pos; }

	Transform transform() const
	{ return transform_; }

	decltype(auto) operator*() const
	{ return transform_(*pos_); }

	TransformIterator& operator++()
	{
		++pos_;
		return *this;
	}

	TransformIterator operator++(int)
	{
		auto cpy = *this;
		++(*this);
		return cpy;
	}

	It operator->() const
	{ return pos_; }

	friend void swap(TransformIterator& left, TransformIterator& right)
	{
		using std::swap;
		swap(left.pos_, right.pos_);
		swap(left.transform_, right.transform_);
	}

protected:
	It pos_;
	Transform transform_;
};

template <class It, class Transform>
struct TransformIterator<It, Transform, std::forward_iterator_tag>:
	public TransformIterator<It, Transform, std::input_iterator_tag>
{
	using base_type = TransformIterator<It, Transform, std::input_iterator_tag>;
	using transform_type    = typename base_type::transform_type;
	using value_type        = typename base_type::value_type;
	using iterator_category = typename base_type::iterator_category;
	using difference_type   = typename base_type::difference_type;
	using reference         = typename base_type::reference;
	using pointer           = typename base_type::pointer;

	using base_type::base_type;	
	TransformIterator() = default;
	TransformIterator(const TransformIterator&) = default;
	TransformIterator(TransformIterator&&) = default;

	TransformIterator& operator=(const TransformIterator&) = default;
	TransformIterator& operator=(TransformIterator&&) = default;
};

template <class It, class Transform>
struct TransformIterator<It, Transform, std::bidirectional_iterator_tag>:
	public TransformIterator<It, Transform, std::forward_iterator_tag>
{
	using base_type = TransformIterator<It, Transform, std::forward_iterator_tag>;
	using transform_type    = typename base_type::transform_type;
	using value_type        = typename base_type::value_type;
	using iterator_category = typename base_type::iterator_category;
	using difference_type   = typename base_type::difference_type;
	using reference         = typename base_type::reference;
	using pointer           = typename base_type::pointer;

	using base_type::base_type;	
	TransformIterator() = default;
	TransformIterator(const TransformIterator&) = default;
	TransformIterator(TransformIterator&&) = default;

	TransformIterator& operator=(const TransformIterator&) = default;
	TransformIterator& operator=(TransformIterator&&) = default;

	TransformIterator& operator--()
	{
		--(this->pos_);
		return *this;
	}

	TransformIterator operator--(int)
	{
		auto cpy = *this;
		--(*this);
		return cpy;
	}
};

template <class It, class Transform>
struct TransformIterator<It, Transform, std::random_access_iterator_tag>:
	public TransformIterator<It, Transform, std::bidirectional_iterator_tag>
{
	using base_type = TransformIterator<It, Transform, std::bidirectional_iterator_tag>;
	using transform_type    = typename base_type::transform_type;
	using value_type        = typename base_type::value_type;
	using iterator_category = typename base_type::iterator_category;
	using difference_type   = typename base_type::difference_type;
	using reference         = typename base_type::reference;
	using pointer           = typename base_type::pointer;

	using base_type::base_type;	
	TransformIterator() = default;
	TransformIterator(const TransformIterator&) = default;
	TransformIterator(TransformIterator&&) = default;

	TransformIterator& operator=(const TransformIterator&) = default;
	TransformIterator& operator=(TransformIterator&&) = default;

	TransformIterator& operator+=(difference_type delta)
	{
		this->base(this->base() + delta);
		return *this;
	}

	friend TransformIterator operator+(difference_type delta, TransformIterator it)
	{ return it += delta; }
		

	friend TransformIterator operator+(TransformIterator it, difference_type delta)
	{ return it += delta; }

	TransformIterator& operator-=(difference_type delta)
	{
		this->base(this->base() - delta);
		return *this;
	}

	friend TransformIterator operator-(TransformIterator it, difference_type delta)
	{ return it -= delta; }

	friend difference_type operator-(const TransformIterator& left, const TransformIterator& right)
	{ return left.base() - right.base(); }

	decltype(auto) operator[](difference_type i) const
	{ return *(*this + i); }

	friend bool operator<(const TransformIterator& left, const TransformIterator& right)
	{ return left.base() < right.base(); }

	friend bool operator<=(const TransformIterator& left, const TransformIterator& right)
	{ return left.base() <= right.base(); }

	friend bool operator>(const TransformIterator& left, const TransformIterator& right)
	{ return left.base() > right.base(); }

	friend bool operator>=(const TransformIterator& left, const TransformIterator& right)
	{ return left.base() >= right.base(); }

};

template <class It, class Transform>
TransformIterator<std::decay_t<It>, std::decay_t<Transform>> make_transform_iterator(It&& pos, Transform&& tform)
{
	return TransformIterator<std::decay_t<It>, std::decay_t<Transform>>(
		std::forward<It>(pos), std::forward<Transform>(tform)
	);
}

} /* namespace wasm */

#endif /* TRANSFORM_ITERATOR_H */

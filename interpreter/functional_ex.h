#ifndef FUNCTIONAL_EX_H
#define FUNCTIONAL_EX_H


#include <functional>
#include <type_traits>
#include "bitutils.h"
#include "utilities/bit_cast.h"

template <class T = void>
struct bit_lshift
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		static_assert(std::is_unsigned_v<T>, "Don't pass signed values to bit_lshift::operator() plz.");
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		return lhs << (rhs % bits); 
	}
};
template <>
struct bit_lshift<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_lshift<T>{}(lhs, rhs); }
};

template <class T = void>
struct bit_rshift
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		const auto shift = rhs % bits;
		if constexpr(std::is_unsigned_v<T>)
			return lhs >> shift; 
		else
		{
			UnsignedT = std::make_unsigned_t<T>;
			UsignedT lhs_u = bit_cast<UnsignedT>(lhs);
			bool msb = (UnsignedT(1) << (bits - 1)) & lhs_u;
			lhs_u >>= shift;
			if(msb)
			{
				// sign extension
				UnsignedT mask = ~UnsignedT(0);
				mask <<= (bits - shift);
				lhs_u |= mask;
			}
			return bit_cast<T>(lhs_u);
		}
	}
};

template <>
struct bit_rshift<void>
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_rshift<T>{}(lhs, rhs); }
};

template <class T>
struct bit_rrotate
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		const auto shift = rhs % bits;
		return (v >> shift) | (v << (bits - shift));
	}
};
template <>
struct bit_rrotate
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_rotate_right<T>{}(lhs, rhs); }
};

template <class T>
struct bit_lrotate
{
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{
		constexpr std::size_t bits = std::numeric_limits<T>::digits;
		const auto shift = rhs % bits;
		return (v << shift) | (v >> (bits - shift));
	}
};
template <>
struct bit_lrotate
{
	template <class T>
	decltype(auto) operator()(const T& lhs, const T& rhs) const
	{ return bit_rotate_right<T>{}(lhs, rhs); }
};

template <class T>
struct bit_clz
{
	decltype(auto) operator()(const T& val) const
	{
		return count_leading_zeros(val);
	}
};

template <>
struct bit_clz
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_clz<T>{}(val); }
};

template <class T>
struct bit_ctz
{
	decltype(auto) operator()(const T& val) const
	{ return count_trailing_zeros(val); }
};

template <>
struct bit_ctz
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_ctz<T>{}(val); }
};

template <class T>
struct bit_popcnt
{
	decltype(auto) operator()(const T& val) const
	{ return population_count(val); }
};

template <>
struct bit_popcnt
{
	template <class T>
	decltype(auto) operator()(const T& val) const
	{ return bit_popcnt<T>{}(val); }
};

#endif /* FUNCTIONAL_EX_H */

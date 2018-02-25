#ifndef BITUTILS_H
#define BITUTILS_H

#include <cstddef>
#include <limits>
#include <type_traits>
#include "utilities/bit_cast.h"

template <class T>
constexpr std::size_t int_width() 
{
	return std::numeric_limits<T>::digits;
}

template <std::size_t LB, std::size_t UB, class T>
std::size_t _bsearch_clz(T value)
{
	if constexpr(UB - LB == 1)
	{
		return UB;
	}
	else
	{
		T tmp = T(1) << (LB + (UB - LB) / 2);
		if(tmp <= value)
			return _bsearch_clz<(LB + (UB - LB) / 2), UB>(value);
		else
			return _bsearch_clz<LB, (LB + (UB - LB) / 2)>(value);
	}
}
template <class T>
std::size_t bsearch_clz(T value)
{
	if(value == T(0))
		return int_width<T>();
	else if(value >= T(1) << (int_width<T>() - 1))
		return 0;
	else
		return int_width<T>() - _bsearch_clz<0, int_width<T>()>(value);
}

template <std::size_t LB, std::size_t UB, class T>
std::size_t _bsearch_ctz(T value)
{
	if constexpr(UB - LB == 1)
	{
		return UB;
	}
	else
	{
		if((value << (LB + (UB - LB) / 2)) > T(0))
			return _bsearch_ctz<(LB + (UB - LB) / 2), UB>(value);
		else
			return _bsearch_ctz<LB, (LB + (UB - LB) / 2)>(value);
	}
}
template <class T>
std::size_t bsearch_ctz(T value)
{
	// binary search on how far to shift<< value to make it zero
	return int_width<T>() - _bsearch_ctz<0, int_width<T>()>(value);

}

template <class T>
std::size_t bsearch_ffs(T value)
{
	if(value == T(0))
		return 0;
	else
		return bsearch_ctz(value) + 1;
}

#ifdef __GNUC__
template <class T>
struct GNU_Intrin
{
	static constexpr const std::size_t bits = 
		std::numeric_limits<std::make_unsigned_t<T>>::digits;
	static constexpr const std::size_t int_bits = 
		std::numeric_limits<unsigned int>::digits;
	static constexpr const std::size_t long_bits = 
		std::numeric_limits<unsigned long>::digits;
	static constexpr const std::size_t long_long_bits = 
		std::numeric_limits<unsigned long long>::digits;
	using UnsignedT = std::make_unsigned_t<T>;
	static constexpr std::size_t clz(T value)
	{
		if constexpr(bits == long_long_bits)
			return value ? __builtin_clzll(value) : bits;
		else if constexpr(bits == long_bits)
			return value ? __builtin_clzl(value) : bits;
		else if constexpr(bits == int_bits)
			return value ? __builtin_clz(value) : bits;
		else 
		{
			static_assert(bits < int_bits);
			if(not value)
				return bits;
			return __builtin_clz(value) - (int_bits - bits);
		}
	}
	static constexpr std::size_t ctz(T value)
	{
		if constexpr(bits == long_long_bits)
			return value ? __builtin_ctzll(value) : bits;
		else if constexpr(bits == long_bits)
			return value ? __builtin_ctzl(value) : bits;
		else if constexpr(bits == int_bits)
			return value ? __builtin_ctz(value) : bits;
		else
		{
			static_assert(bits < int_bits);
			if(not value)
				return bits;
			return __builtin_ctz(value);
		}
	}
	static constexpr std::size_t popcount(T value)
	{
		if constexpr(bits == long_long_bits)
			return __builtin_popcount(value);
		else if constexpr(bits == long_bits)
			return __builtin_popcount(value);
		else
		{
			// we want to avoid any problems with sign extension here
			// so we'll memcpy in the bits of 'value'
			static_assert(bits <= int_bits);
			unsigned int value_cpy = 0;
			std::memcpy(&value_cpy, &value, sizeof(value));
			return __builtin_popcount(value_cpy);
		}
	}
};
#endif 

template <class T>
std::size_t count_leading_zeros(T value)
{
#ifndef __GNUC__
	return bsearch_clz(value);
#else
	return GNU_Intrin<T>::clz(value);
#endif
}

template <class T>
std::size_t count_trailing_zeros(T value)
{
#ifndef __GNUC__
	return bsearch_ctz(value);
#else
	return GNU_Intrin<T>::ctz(value);
#endif
}

template <class T>
std::size_t population_count(T value)
{
#ifndef __GNUC__
	if constexpr(std::is_signed_v<T>)
	{
		using UnsignedT = std::make_unsigned_t<T>;
		std::size_t count = 0;
		UnsignedT uvalue = bit_cast<UnsignedT>(value);
		while(uvalue)
		{
			count += uvalue & 0x01;
			uvalue >>= 1;
		}
		return count;
	}
	else
	{
		std::size_t count = 0;
		while(value)
		{
			count += value & 0x01;
			value >>= 1;
		}
		return count;
	}
#else
	return GNU_Intrin<T>::popcount(value);
#endif
}


#endif /* BITUTILS_H */

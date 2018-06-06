#ifndef UTILITIES_ENDIANNESS_H
#define UTILITIES_ENDIANNESS_H

#include <climits>
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace wasm {

inline bool system_is_little_endian()
{
	std::uintmax_t one = 1u;
	alignas(std::uintmax_t) char buff[sizeof(std::uintmax_t)];
	std::memcpy(buff, &one, sizeof(one));
	return buff[0] == 1;
}

template <class T>
T byte_swap(T value)
{
	static_assert(std::is_trivially_copyable_v<T>);
	if constexpr(sizeof(value) == 1)
	{
		return value;
	}
#ifdef __GNUC__
	// clang code generation is better when using __builtin_bswap
	else if constexpr(sizeof(value) == sizeof(std::uint16_t))
	{
		std::uint16_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		tmp = __builtin_bswap16(tmp);
		std::memcpy(&value, &tmp, sizeof(tmp));
		return value;
	}
	else if constexpr(sizeof(value) == sizeof(std::uint32_t))
	{
		std::uint32_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		tmp = __builtin_bswap32(tmp);
		std::memcpy(&value, &tmp, sizeof(tmp));
		return value;
	}
	else if constexpr(sizeof(value) == sizeof(std::uint64_t))
	{
		std::uint64_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		tmp = __builtin_bswap64(tmp);
		std::memcpy(&value, &tmp, sizeof(tmp));
		return value;
	}
#endif /* __GNUC__ */
	else
	{
		alignas(alignof(value)) char buff[sizeof(value)];
		std::memcpy(bytes, &value, sizeof(value));
		std::reverse(bytes, bytes + sizeof(value));
		std::memcpy(&value, bytes, sizeof(value));
		return value;
	}
}

template <bool Signed, class SrcCharT, class DestCharT>
inline void le_to_be(const SrcCharT* src_begin, const SrcCharT* src_end, DestCharT* dest_begin, DestCharT* dest_end)
{
	std::size_t srclen = src_end - src_begin;
	std::size_t destlen = dest_end - dest_begin;
	std::size_t len = std::min(srclen, destlen);
	auto dest_rbegin = std::make_reverse_iterator(dest_end);
	auto dest_rend = std::make_reverse_iterator(dest_begin);
	dest_rbegin = std::copy(src_begin, src_begin + len, dest_rbegin);
	char fillvalue = 0;
	if(Signed and srclen < destlen) 
	{
		fillvalue = ((unsigned char)(src_begin[srclen - 1]) >> (CHAR_BIT - 1));
		if(fillvalue > 0)
			fillvalue = ~(unsigned char)(0);
	}
	std::fill(dest_rbegin, dest_rend, fillvalue);
}

template <bool Signed, class SrcCharT, class DestCharT>
inline void be_to_le(const SrcCharT* src_begin, const SrcCharT* src_end, DestCharT* dest_begin, DestCharT* dest_end)
{
	std::size_t srclen = src_end - src_begin;
	std::size_t destlen = dest_end - dest_begin;
	std::size_t len = std::min(srclen, destlen);
	auto src_rbegin = std::make_reverse_iterator(src_end);
	// auto src_rend = std::make_reverse_iterator(src_begin);
	dest_begin = std::copy(src_rbegin, src_rbegin + len, dest_begin);
	char fillvalue = 0;
	if(Signed and srclen < destlen) 
	{
		fillvalue = ((unsigned char)(src_begin[0])) >> (CHAR_BIT - 1);
		if(fillvalue > 0)
			fillvalue = ~(unsigned char)(0);
	}
	std::fill(dest_begin, dest_end, fillvalue);
}

template <class Integer>
Integer le_to_system(Integer value)
{
	static_assert(std::is_integral_v<Integer>, "Endianness only affects integral types.");
	if constexpr(sizeof(Integer) > 1)
	{
		if(is_big_endian())
			byte_swap(value);
	}
	return value;
}

template <class Integer>
Integer be_to_system(Integer value)
{
	static_assert(std::is_integral_v<Integer>, "Endianness only affects integral types.");
	if constexpr(sizeof(Integer) > 1)
	{
		if(not is_big_endian())
			byte_swap(value);
	}
	return value;
}

} /* namespace wasm */

#endif /* UTILITIES_ENDIANNESS_H */

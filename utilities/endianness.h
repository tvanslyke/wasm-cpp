#ifndef UTILITIES_ENDIANNESS_H
#define UTILITIES_ENDIANNESS_H

#include <climits>
#include <cstdint>
#include <algorithm>

inline bool is_big_endian()
{
	static_assert(CHAR_BIT == 8);
	union {
		std::uint_least32_t integer;
		char bytes[4];
	} value;
	value.integer = 0x01020304;
	return value.c[0] == 1; 
}

template <class T>
void byte_swap(T& value)
{
	char bytes[sizeof(value)];
	std::memcpy(bytes, &value, sizeof(value));
	std::rotate(bytes, bytes + sizeof(value));
	std::memcpy(&value, bytes, sizeof(value));
}

template <bool Signed = false>
inline void le_to_be(char* src_begin, char* src_end, char* dest_begin, char* dest_end)
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

template <bool Signed = false>
inline void be_to_le(char* src_begin, char* src_end, char* dest_begin, char* dest_end)
{
	std::size_t srclen = src_end - src_begin;
	std::size_t destlen = dest_end - dest_begin;
	std::size_t len = std::min(srclen, destlen);
	auto src_rbegin = std::make_reverse_iterator(src_end);
	auto src_rend = std::make_reverse_iterator(src_begin);
	dest_begin = std::copy(src_rbegin, src_rbegin + len, dest_rbegin);
	char fillvalue = 0;
	if(Signed and srclen < destlen) 
	{
		fillvalue = ((unsigned char)(src_begin[0])) >> (CHAR_BIT - 1);
		if(fillvalue > 0)
			fillvalue = ~(unsigned char)(0);
	}
	std::fill(dest_rbegin, dest_rend, fillvalue);
}


#endif /* UTILITIES_ENDIANNESS_H */

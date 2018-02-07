#ifndef LEB_128_H
#define LEB_128_H

#include "wasm_base.h"
#include "utilities/endianness.h"
#include "utilities/bit_cast.h"
#include <cstdint>
#include <tuple>

// honestly idk why I'm bothering
static constexpr bool is_twos_comp()
{
	return (~signed(0)) == -signed(1);
}

template <class Integer, class CharIt>
using leb128_result = std::tuple<Integer, CharIt, std::size_t>;

template <class UIntType, class CharIt>
[[nodiscard]]
leb128_result<UIntType, CharIt>
leb128_decode_uint(CharIt begin, CharIt end)
{
	using byte_t = std::uint_least8_t;
	constexpr byte_t mask = 0b01111111;
	constexpr std::size_t bitcount = std::numeric_limits<UIntType>::digits;
	
	static_assert(std::is_unsigned_v<UIntType>);
	assert(begin < end);

	UIntType value{0};
	std::size_t count = 0;
	std::size_t shift = 0;
	for(byte_t byte_v = 0; begin < end and (byte_v & (~mask)); ++count, (void)(byte_v = *begin++))
	{
		assert(shift < bitcount);
		value |= UIntType(mask & byte_v) << shift;
		shift += 7;
	}
	return {value, begin, count};
}

template <class IntType, class CharIt>
[[nodiscard]]
leb128_result<IntType, CharIt>
leb128_decode_sint(CharIt begin, CharIt end)
{
	using UIntType = std::make_unsigned_t<IntType>;
	using byte_t = std::uint_least8_t;
	constexpr byte_t mask = 0b01111111;
	constexpr std::size_t bitcount = std::numeric_limits<UIntType>::digits;
	
	// preconditions
	static_assert(std::is_signed_v<IntType>);
	assert(begin < end);

	UIntType value{0};
	IntType result = 0;
	std::size_t count = 0;
	std::size_t shift = 0;
	byte_t byte_v = 0;
	for(; begin < end and (byte_v & (~mask)); ++count, (void)(byte_v = *begin++))
	{
		assert(shift < bitcount);
		value |= wasm_uint64_t(mask & byte_v) << shift;
		shift += 7;
	}
	if((shift < bitcount) and (byte_v & std::uint_least8_t(0b01000000)))
		value |= - (1 << shift);

	// value is now twos-comp repr
	if constexpr(not is_twos_comp())
	{
		// honestly, I should just require two's complement but ...
		if(value & (UIntType(1) << (bitcount - 1)))
			value = (~value) + 1;
		result = -1 * IntType(value);
	}
	else
		result = bit_cast<IntType>(value);
	return {result, begin, count};
}

template <class Integer, class CharIt>
[[nodiscard]]
leb128_result<Integer, CharIt> 
leb128_decode(CharIt begin, CharIt end)
{
	if constexpr(std::is_unsigned_v<Integer>)
		return leb128_decode_uint<Integer>(begin, end);
	else if constexpr(std::is_signed_v<Integer>)
		return leb128_decode_sint<Integer>(begin, end);
	else
		static_assert(std::is_signed_v<Integer> or std::is_unsigned_v<Integer>,
				"Something's not right.  Attempt to use type in "
				"'leb128_decode()' which is neither signed nor unsigned.");
}


template <class Integer>
struct LEB128_Decoder
{
	template <class CharIt>
	[[nodiscard]]
	leb128_result<Integer, CharIt> operator()(CharIt begin, CharIt end) const
	{
		auto [value, iter, count] = leb128_decode<Integer>(begin, end);
		// convert to big endian if system is not little endian
		return {le_to_system(value), iter, count};
	}
};

static const LEB128_Decoder<std::uint_least8_t> leb128_decode_uint7;
static const LEB128_Decoder<std::uint_least8_t> leb128_decode_uint8;
static const LEB128_Decoder<std::uint_least16_t> leb128_decode_uint16;
static const LEB128_Decoder<std::uint_least32_t> leb128_decode_uint32;
static const LEB128_Decoder<std::uint_least64_t> leb128_decode_uint64;

static const LEB128_Decoder<std::int_least8_t> leb128_decode_sint7;
static const LEB128_Decoder<std::int_least8_t> leb128_decode_sint8;
static const LEB128_Decoder<std::int_least16_t> leb128_decode_sint16;
static const LEB128_Decoder<std::int_least32_t> leb128_decode_sint32;
static const LEB128_Decoder<std::int_least64_t> leb128_decode_sint64;


template <class CharIt>
[[nodiscard]]
leb128_result<std::uint_least8_t, CharIt>
leb128_decode_uint1(CharIt begin, [[maybe_unused]] CharIt end)
{
	assert(begin != end);
	auto v = *begin++;
	assert(not (v & std::uint_least8_t(0b10000000)) and "Attempt to decode 1-bit leb128, but there are more bytes to parse!");
	assert(not (v & std::uint_least8_t(0b01111110)) and "Attempt to decode 1-bit leb128, but non-least-significant-bits are set!");
	return {v & 0x01, begin, 1};
}

#endif /* LEB_128_H */

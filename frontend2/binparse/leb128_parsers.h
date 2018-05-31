#ifndef BINPARSE_LEB128_PARSERS_H
#define BINPARSE_LEB128_PARSERS_H

#include <utility>
#include <cstring>
#include "../../include/wasm_base.h"
#include "../../include/WasmInstruction.h"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
namespace wasm::parse {

namespace x3 = boost::spirit::x3;

template <class Unsigned, std::size_t BitCount = sizeof(Unsigned) * CHAR_BIT>
struct UnsignedLeb128Parser:
	public x3::parser<UnsignedLeb128Parser<Unsigned>>
{
	static_assert(std::is_unsigned_v<Unsigned>);
	using attribute_type = Unsigned;
	static constexpr const bool has_attribute = true;

        template <typename It, typename Context, typename Other, typename Attribute>
        bool parse(It& first_, It const& last, const Context&, Other const&, Attribute& attr) const
	{
		auto first = first_;
		Unsigned result{0};
		constexpr std::size_t bit_count = BitCount;
		std::uint8_t byte;
		for(std::size_t sh = 0; ; sh += 7)
		{
			if((first == last) or (sh >= bit_count))
			{
				return false;
			}
			auto c = *first++;
			assert(sizeof(c) == sizeof(byte));
			std::memcpy(&byte, &c, sizeof(byte));
			result |= static_cast<Unsigned>(byte & 0b0111'1111u) << sh;
			if(not static_cast<bool>(byte & 0b1000'0000u))
				break;
		}
		attr = result;
		first_ = first;
		return true;
	}
};

template <class Signed, std::size_t BitCount = sizeof(Signed) * CHAR_BIT>
struct SignedLeb128Parser:
	public x3::parser<SignedLeb128Parser<Signed>>
{
	static_assert(std::is_integral_v<Signed> and std::is_signed_v<Signed>);
	using attribute_type = Signed;
	static constexpr const bool has_attribute = true;

        template <typename It, typename Context, typename Other, typename Attribute>
        bool parse(It& first_, It const& last, const Context&, Other const&, Attribute& attr) const
	{
		using Unsigned = std::make_unsigned_t<Signed>;
		auto first = first_;
		Signed result{0};
		constexpr std::size_t bit_count = BitCount;
		Unsigned byte;
		std::size_t shift = 0;
		do {
			if(shift >= bit_count)
				return false;
			byte = static_cast<Unsigned>(*first++);
			result |= (byte & Unsigned(0b0111'1111u)) << shift;
			shift += 7;
		} while(static_cast<bool>(byte & Unsigned(0b1000'0000u)));
		if((shift < bit_count) and static_cast<bool>(byte & Unsigned(0b0100'0000u)))
			result |= (~Unsigned(0)) << shift;
		attr = static_cast<Signed>(result);
		first_ = first;
		return true;
	}
};

inline const auto varuint1 = x3::rule<struct VarUInt1, bool>{}
	= (x3::byte_(0u) >> x3::attr(false)) | (x3::byte_(1u) >> x3::attr(true));

inline const auto varuint7 = x3::rule<struct VarUInt7, std::uint8_t>{}
	= UnsignedLeb128Parser<std::uint8_t, 7>{};

inline const auto varuint32 = x3::rule<struct VarUInt32, std::uint32_t>{}
	= UnsignedLeb128Parser<std::uint32_t>{};

inline const auto varint7 = x3::rule<struct VarInt7, std::int8_t>{}
	= SignedLeb128Parser<std::int8_t, 7>{};

inline const auto varint32 = x3::rule<struct VarInt32, std::int32_t>{}
	= SignedLeb128Parser<std::int32_t>{};

inline const auto varint64 = x3::rule<struct VarInt64, std::int64_t>{}
	= SignedLeb128Parser<std::int64_t>{};

template <class ItemParser>
struct Varuint32PrefixedSequence: 
	public x3::unary_parser<ItemParser, Varuint32PrefixedSequence<ItemParser>>
{
	using base_type = x3::unary_parser<ItemParser, Varuint32PrefixedSequence<ItemParser>>;
	static constexpr const bool handles_container = true;
	static constexpr const bool is_pass_through_unary = true;

	Varuint32PrefixedSequence(ItemParser ip):
		base_type(std::move(ip))
	{
		
	}

	template <typename It, typename Ctx, typename Other, typename Attr>
        bool parse(It& f, It l, Ctx&& ctx, const Other & other, Attr& attr) const 
	{
		auto pos = f;
		std::uint32_t len{0};
		if(not varuint32.parse(pos, l, ctx, other, len))
			return false;
		if(len)
		{
			if(not (x3::repeat(len)[this->subject]).parse(pos, l, ctx, other, attr))
				return false;
		}
		f = pos; 
		return true;
	}	
};

template <class Parser>
auto varuint32_prefixed_sequence(Parser&& parser)
{
	using varuint32_t = std::decay_t<decltype(varuint32)>;
	return Varuint32PrefixedSequence<std::decay_t<Parser>>(
		std::forward<Parser>(parser)
	);
}

} /* namespace wasm::parse */

#endif /* BINPARSE_LEB128_PARSERS_H */

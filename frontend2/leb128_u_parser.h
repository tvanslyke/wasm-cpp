#ifndef LEB128_U_PARSER_H 
#define LEB128_U_PARSER_H 
#include <iostream>
#include <cstdint>
#include <type_traits>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <string>
#include <stdexcept>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>


namespace wasm::parse { 
BOOST_SPIRIT_TERMINAL(leb128_u);
}

namespace boost::spirit
{
	template <>
	struct use_terminal<qi::domain, wasm::parse::tag::leb128_u>: mpl::true_
	{};
} /* namespace boost::spirit */

namespace wasm::parse
{
    struct leb128_u_parser
      : boost::spirit::qi::primitive_parser<leb128_u_parser>
    {
        // Define the attribute type exposed by this parser component
        template <typename Context, typename It>
        struct attribute
        {
		using type = std::uintmax_t;
        };
 
        // This function is called during the actual parsing process
        template <typename It, typename Context, typename Skipper, typename Attribute>
        bool parse(It& first, It const& last, Context&, Skipper const& skipper, Attribute& attr) const
        {
		boost::spirit::qi::skip_over(first, last, skipper);
		std::uintmax_t result{0};
		constexpr std::size_t bit_count = sizeof(result) * CHAR_BIT;
		std::uint8_t byte;
		for(std::size_t sh = 0; ; sh += 7)
		{
			if(first == last or sh >= bit_count)
				return false;
			auto c = *first++;
			assert(sizeof(c) == sizeof(byte));
			std::memcpy(&byte, &c, sizeof(byte));
			result |= static_cast<std::uintmax_t>(byte & 0b0111'1111u) << sh;
			if(not static_cast<bool>(byte & 0b1000'0000u))
				break;
		}
		attr = result;
		return true;
        }
 
        // This function is called during error handling to create
        // a human readable string for the error context.
        template <typename Context>
        boost::spirit::info what(Context&) const
        {
		return boost::spirit::info("leb128 unsigned");
        }
    };
}

namespace boost::spirit::qi {
	// This is the factory function object invoked in order to create
	// an instance of our iter_pos_parser.
	template <typename Modifiers>
	struct make_primitive<wasm::parse::tag::leb128_u, Modifiers>
	{
		typedef wasm::parse::leb128_u_parser result_type;

		result_type operator()(unused_type, unused_type) const
		{
			return result_type();
		}
	};
}




#endif /* LEB128_U_PARSER_H */

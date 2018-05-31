#ifndef BINPARSE_HELPERS_H
#define BINPARSE_HELPERS_H

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include <utility>
#include <type_traits>

namespace wasm::parse {

namespace x3 = boost::spirit::x3;

template <typename T>
struct as_type
{
	template <typename Expr>
	auto operator[](Expr&& expr) const
	{
		return x3::rule<struct _, T>{"as"} = x3::as_parser(std::forward<Expr>(expr));
	}
};

template <typename T>
inline const as_type<T> as = {};

namespace detail {

template <class Expr, class T = void>
struct disable_if_fails {
	using type = T;
};

template <class Expr, class T = void>
using disable_if_fails_t = typename disable_if_fails<Expr, T>::type;

} /* namespace detail */

template <
	class Parser,
	class Type = std::enable_if_t<
		x3::traits::is_parser<Parser>::value,
		detail::disable_if_fails_t<typename Parser::attribute_type>
	>
>
struct x3_attribute_type {
	using type = typename Parser::attribute_type;
};

template <class Parser>
using x3_attribute_type_t = typename x3_attribute_type<Parser>::type;

template <class OutAttrType, class Parser, class Transform>
auto transform_attr(Parser&& parser, Transform&& trans)
{
	return x3::rule<struct transform_attr_tag_, OutAttrType>{}
		%= as<OutAttrType>[(std::forward<Parser>(parser)[(
			[trans = std::forward<Transform>(trans)](auto& ctx){ x3::_val(ctx) = trans(x3::_attr(ctx)); }
		)])];
}

template <
	class Parser, 
	class Transform, 
	class InAttrType = x3_attribute_type_t<Parser>
>
auto transform_attr(Parser&& parser, Transform&& trans)
{
	using out_attr_type = std::invoke_result_t<Transform, InAttrType>;
	return transform_attr<out_attr_type>(std::forward<Parser>(parser), std::forward<Transform>(trans));
}

template <class Tag_, class AttrType, bool Forced, class Transform>
auto transform_attr(const x3::rule<Tag_, AttrType, Forced>& parser, Transform&& trans)
{
	static_assert(
		not std::is_same_v<AttrType, x3::unused_type>,
		"Cannot transform 'unused_type' in 'transform_attr()'."
	);
	return transform_attr<AttrType>(parser, std::forward<Transform>(trans));
}

template <class Tag_, class RHS, class AttrType, bool Forced, class Transform>
auto transform_attr(const x3::rule_definition<Tag_, RHS, AttrType, Forced>& parser, Transform&& trans)
{
	static_assert(
		not std::is_same_v<AttrType, x3::unused_type>,
		"Cannot transform 'unused_type' in 'transform_attr()'."
	);
	return transform_attr<AttrType>(parser, std::forward<Transform>(trans));
}

template <class Parser>
struct MatchWithoutConsuming:
	public x3::unary_parser<Parser, MatchWithoutConsuming<Parser>>
{
	using base_type = x3::unary_parser<Parser, MatchWithoutConsuming<Parser>>;
	static constexpr const bool is_pass_through_unary = true;

	using base_type::base_type;
	
        template <typename It, typename Context, typename Other, typename Attribute>
        bool parse(It first, It last, const Context& ctx, Other const& other, Attribute& attr) const
	{
		return this->subject.parse(first, last, ctx, other, attr);
	}
};

template <class Parser>
MatchWithoutConsuming<Parser> match_without_consuming(const Parser& parser)
{ return MatchWithoutConsuming<Parser>(parser); }

template <class It>
struct expectation_failure:
	public x3::expectation_failure<It>
{
	expectation_failure(It pos, const std::string& msg):
		x3::expectation_failure<It>(pos, msg)
	{
		static_cast<std::runtime_error&>(*this) = std::runtime_error(msg);
	}
};

template <class AsType, class Parser, class T>
auto match_value(Parser parser, T value)
{
	return x3::omit[parser(static_cast<AsType>(value))] >> x3::attr(value);
}

template <class Parser, class Match, class Attr>
auto match_value(Parser parser, Match match_v, Attr attr_v)
{
	return x3::omit[parser(match_v)] >> x3::attr(attr_v);
}

template <class T>
auto varuint1_option(T&& value) 
{
	return x3::omit[x3::byte_(0)] | (x3::omit[x3::byte_(1)] >> std::forward<T>(value));
};

} /* namespace wasm::parse */

#endif /* BINPARSE_HELPERS_H */

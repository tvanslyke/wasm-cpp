#include <boost/spirit/home/x3.hpp>
#include <type_traits>
#include <iostream>
#include <numeric>

namespace x3 = boost::spirit::x3;

namespace std {
    template <typename T> // just for easy debug printing; hack
    static std::ostream& operator<<(std::ostream& os, std::vector<T> const& v) {
        for (auto& el : v) std::cout << '[' << el << ']';
        return os;
    }
}

using string_vec  = std::vector<std::string>;
using result_type = boost::variant<std::string, double, string_vec>;

template <typename Parser>
void parse(const std::string message, const std::string &input, const std::string &rule, const Parser &parser) {
    auto iter = input.begin(), end = input.end();

    std::cout << "-------------------------\n";
    std::cout << message << "\n";
    std::cout << "Rule:     " << rule  << "\n";
    std::cout << "Parsing: '" << input << "'\n";

    result_type parsed_result;
    bool result = phrase_parse(iter, end, parser, x3::space, parsed_result);

    if (result) {
        std::cout << "Parsed " << parsed_result << "\n";
    } else {
        std::cout << "Parser failed\n";
    }
    if (iter != end)
        std::cout << "EOI not reached. Unparsed: '" << std::string(iter, end) << "'\n";
}

namespace parser {

    template <typename... Parsers>
    struct longest_parser : x3::parser_base {
        longest_parser(Parsers... sub) : _alternatives {sub...} { }

        template <typename It, typename Ctx, typename Other, typename Attr>
        bool parse(It& f, It l, Ctx& ctx, Other const& other, Attr& attr) const {
            auto const saved = f;

            //// To exclude pre-skip from length comparisons, do pre-skip here:
            // x3::skip_over(f, l, ctx);
            auto seq = std::make_index_sequence<sizeof...(Parsers)>();

            auto best = select_best(f, l, ctx, seq);
            //std::cout << "Longest match at index #" << best << "\n";

            bool ok = dispatch(f, l, ctx, other, attr, best, seq);

            if (!ok)
                f = saved;

            return ok;
        }

      private:
        template <typename It, typename Ctx, typename P>
        size_t length_of(It f, It l, Ctx ctx, P const& p) const {
            boost::iterator_range<It> matched;
            return x3::raw[p].parse(f, l, ctx, x3::unused, matched)? boost::size(matched) : 0;
        }

        template <typename It, typename Ctx, size_t... I>
            size_t select_best(It f, It l, Ctx& ctx, std::index_sequence<I...>) const {
                std::array<size_t, sizeof...(I)> lengths { length_of(f, l, ctx, std::get<I>(_alternatives))... };
                return std::distance(lengths.begin(), std::max_element(lengths.begin(), lengths.end()));
            }

        template <typename It, typename Ctx, typename Other, typename Attr, size_t... I>
        bool dispatch(It& f, It l, Ctx& ctx, Other const& other, Attr& attr, size_t targetIdx, std::index_sequence<I...>) const {
            //return (real_parse<I>(f, l, ctx, other, attr, targetIdx) || ...);
            std::array<bool, sizeof...(I)> b = { real_parse<I>(f, l, ctx, other, attr, targetIdx)... };

            return std::accumulate(b.begin(), b.end(), false, std::logical_or<bool>());
        }

        template <size_t Idx, typename It, typename Ctx, typename Other, typename Attr>
        bool real_parse(It& f, It l, Ctx& ctx, Other const& other, Attr& attr, size_t targetIdx) const {
            if (targetIdx != Idx)
                return false;

            return std::get<Idx>(_alternatives).parse(f, l, ctx, other, attr);
        }

        std::tuple<Parsers...> _alternatives;
    };

    template <typename... Ps>
        longest_parser<Ps...> longest(Ps... p) { return {x3::as_parser(p)...}; }
}

int main() {
    auto id        = x3::rule<void, std::string> {} = x3::lexeme [ x3::char_("a-zA-Z_") >> *x3::char_("a-zA-Z0-9_") ];
    auto qualified = x3::rule<void, string_vec>  {} = id % "::";

#define TEST_CASE(label, input, rule) parse(label, input, #rule, rule)
    TEST_CASE("unqualified"                , "willy"                , parser::longest(id, x3::int_, x3::double_));
    TEST_CASE("unqualified with whitespace", " willy \t"            , parser::longest(id, x3::int_, x3::double_));
    TEST_CASE("integral or number"         , "123.78::anton::lutz"  , parser::longest(id, x3::int_, x3::double_));
    TEST_CASE("qualified"                  , "willy anton::lutz"    , parser::longest(id, x3::int_, x3::double_));
    TEST_CASE("qualified with whitespace"  , "willy \tanton::lutz"  , parser::longest(id, x3::int_, x3::double_));

    TEST_CASE("unqualified"                , "willy"                , parser::longest(id, x3::int_, x3::double_, qualified));
    TEST_CASE("unqualified with whitespace", " willy \t"            , parser::longest(id, x3::int_, x3::double_, qualified));
    TEST_CASE("integral or number"         , "123.78::anton::lutz"  , parser::longest(id, x3::int_, x3::double_, qualified));
    TEST_CASE("qualified"                  , "willy::anton::lutz"   , parser::longest(id, x3::int_, x3::double_, qualified));
    TEST_CASE("qualified with whitespace"  , "willy ::\tanton::lutz", parser::longest(id, x3::int_, x3::double_, qualified));

    TEST_CASE("unqualified"                , "willy"                , parser::longest(x3::int_, x3::double_, qualified));
    TEST_CASE("unqualified with whitespace", " willy \t"            , parser::longest(x3::int_, x3::double_, qualified));
    TEST_CASE("integral or number"         , "123.78::anton::lutz"  , parser::longest(x3::int_, x3::double_, qualified));
    TEST_CASE("qualified"                  , "willy::anton::lutz"   , parser::longest(x3::int_, x3::double_, qualified));
    TEST_CASE("qualified with whitespace"  , "willy ::\tanton::lutz", parser::longest(x3::int_, x3::double_, qualified));
}

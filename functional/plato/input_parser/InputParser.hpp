#ifndef PLATO_INPUT_PARSER_INPUTPARSER
#define PLATO_INPUT_PARSER_INPUTPARSER

#include <boost/phoenix/core.hpp>
#include <boost/phoenix/fusion.hpp>
#include <boost/phoenix/object.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <filesystem>
#include <type_traits>

#include "plato/input_parser/BlockStructRule.hpp"

namespace plato::input_parser
{
namespace detail
{
/// Helper to provide the `value_type` of a std::vector or the type T if it is not a std::vector.
template <typename T>
struct TypeOrVectorValueType
{
    using type = T;
};

template <typename T, typename A>
struct TypeOrVectorValueType<std::vector<T, A>>
{
    using type = typename std::vector<T, A>::value_type;
};

template <typename T>
struct TypeOrVectorValueType<boost::optional<T>>
{
    using type = T;
};

/// A helper class template for obtaining the block structure type associated with
/// a given index of ParsedInput. The BlockStruct type will be the `type` alias.
template <std::size_t Index>
struct InputTypeAt
{
    using plato_input_type = typename boost::fusion::result_of::value_at_c<ParsedInput, Index>::type;
    using type = typename TypeOrVectorValueType<plato_input_type>::type;
};

template <std::size_t Index>
using InputTypeAtT = typename InputTypeAt<Index>::type;

/// A helper alias template for specifying the type of rule associated with an @a InputStruct.
template <typename Iterator, typename InputStruct>
using RuleTypeT = BlockStructRule<Iterator, InputStruct>;

/// Function mainly provides the type needed to construct a tuple.
template <typename Iterator, std::size_t... Is>
auto plato_input_rule_tuple_impl(std::integer_sequence<std::size_t, Is...>)
{
    return std::make_tuple(RuleTypeT<Iterator, InputTypeAtT<Is>>{}...);
}

template <typename Iterator>
auto plato_input_rule_tuple()
{
    constexpr auto tNumPlatoBlockRules = boost::fusion::result_of::size<ParsedInput>::value;
    return plato_input_rule_tuple_impl<Iterator>(std::make_index_sequence<tNumPlatoBlockRules>{});
}

template <typename BSR, std::size_t I>
auto block_semantic_action()
{
    namespace bp = boost::phoenix;
    if constexpr (BSR::kIsNamedBlockStructRule)
    {
        // For named block structs, we'll have a vector of blocks,
        // so push back the result.
        return bp::push_back(bp::at_c<I>(bsq::_val), bsq::_1);
    }
    else
    {
        // Unnamed blocks are not vectors, so just assign the result
        return bp::at_c<I>(bsq::_val) = bsq::_1;
    }
}

template <std::size_t I, typename AllBlockRules>
using BlockRuleTypeAt = typename std::tuple_element<I, AllBlockRules>::type;

template <typename Iterator, typename AllBlockRules, std::size_t... Is>
auto full_block_or_rule_impl(const AllBlockRules& aAllBlockRules, std::integer_sequence<std::size_t, Is...>)
    -> boost::spirit::qi::rule<Iterator, ParsedInput(), boost::spirit::ascii::space_type>
{
    namespace bp = boost::phoenix;
    bsq::rule<Iterator, ParsedInput(), bsa::space_type> tRule =
        *((std::get<Is>(aAllBlockRules).mBlockRule[block_semantic_action<BlockRuleTypeAt<Is, AllBlockRules>, Is>()] |
           ...));
    return tRule;
}
}  // namespace detail

/// A tuple type containing all rules that parse all defined input blocks in ParsedInput
template <typename Iterator>
using ParsedInputRuleTuple = decltype(detail::plato_input_rule_tuple<Iterator>());

template <typename Iterator, typename AllBlockRules>
auto full_block_or_rule(const AllBlockRules& aAllBlockRules)
    -> boost::spirit::qi::rule<Iterator, ParsedInput(), boost::spirit::ascii::space_type>
{
    constexpr auto tNumPlatoBlockRules = std::tuple_size_v<AllBlockRules>;
    return detail::full_block_or_rule_impl<Iterator>(aAllBlockRules, std::make_index_sequence<tNumPlatoBlockRules>{});
}

template <typename Iterator>
struct InputParser : boost::spirit::qi::grammar<Iterator, ParsedInput(), boost::spirit::ascii::space_type>
{
    InputParser() : InputParser::base_type(mStartParsedInput, "Plato")
    {
        namespace bsq = boost::spirit::qi;
        namespace bp = boost::phoenix;
        mStartParsedInput = full_block_or_rule<Iterator>(mAllParsedInputRules);

        bsq::on_error<bsq::fail>(
            mStartParsedInput, std::cout << bp::val("Error! Expecting ") << bsq::_4 << bp::val(" here: \"")
                                         << bp::construct<std::string>(bsq::_3, bsq::_2) << bp::val("\"") << std::endl);
    }

    const ParsedInputRuleTuple<Iterator> mAllParsedInputRules{};

    using Rule = boost::spirit::qi::rule<Iterator, ParsedInput(), boost::spirit::ascii::space_type>;
    Rule mStartParsedInput;
};

/// @brief Parses all content of @a aInput as if it were an input deck.
[[nodiscard]] ParsedInput parse_input(std::string_view aInput);

/// @brief Parses all content of the file @a aFileName.
[[nodiscard]] ParsedInput parse_input_from_file(const std::filesystem::path& aFileName);
}  // namespace plato::input_parser

#endif

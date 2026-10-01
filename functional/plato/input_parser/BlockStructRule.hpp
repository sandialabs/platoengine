#ifndef PLATO_INPUT_PARSER_BLOCKSTRUCTRULE
#define PLATO_INPUT_PARSER_BLOCKSTRUCTRULE

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/phoenix.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <boost/spirit/include/qi.hpp>
#include <tuple>
#include <utility>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/EnumParser.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/Skipper.hpp"

namespace plato::input_parser
{
namespace bsq = boost::spirit::qi;

namespace detail
{
template <typename BlockStruct, std::size_t kIndex>
struct MemberTypeAt
{
    using type = typename boost::fusion::result_of::value_at_c<BlockStruct, kIndex>::type;
};

template <typename BlockStruct, std::size_t kIndex>
auto member_name() -> std::string
{
    return boost::fusion::extension::struct_member_name<BlockStruct, kIndex>::call();
}

template <typename Iterator, typename BlockStruct, std::size_t kIndex>
using RuleAtIndex =
    boost::spirit::qi::rule<Iterator, typename MemberTypeAt<BlockStruct, kIndex>::type(), SkipperType<Iterator>>;

template <typename Iterator, typename BlockStruct, std::size_t kIndex>
auto rule_at_index() -> RuleAtIndex<Iterator, BlockStruct, kIndex>
{
    const RuleAtIndex<Iterator, BlockStruct, kIndex> tRule = bsq::lit(member_name<BlockStruct, kIndex>()) > bsq::auto_;
    return tRule;
}

template <typename Iterator, typename BlockStruct, std::size_t... kIs>
auto rule_tuple_impl(std::integer_sequence<std::size_t, kIs...> /*unused*/)
{
    return std::make_tuple(rule_at_index<Iterator, BlockStruct, kIs>()...);
}
}  // namespace detail

template <typename Iterator, typename BlockStruct>
auto rule_tuple()
{
    constexpr auto tNumRules = boost::fusion::result_of::size<BlockStruct>::value;
    return detail::rule_tuple_impl<Iterator, BlockStruct>(std::make_index_sequence<tNumRules>{});
}

/// A tuple type containing all key-value-pair rules defined in type BlockStruct
template <typename Iterator, typename BlockStruct>
using BlockRuleTuple = decltype(rule_tuple<Iterator, BlockStruct>());

namespace detail
{
template <typename Iterator>
using NameRule = bsq::rule<Iterator, std::string(), SkipperType<Iterator>>;

template <typename Iterator>
[[nodiscard]] auto name_identifier_rule() -> NameRule<Iterator>
{
    constexpr auto tValidIdentifierChars = "-a-zA-Z0-9_";
    return NameRule<Iterator>{bsq::lexeme[+bsq::char_(tValidIdentifierChars)]};
}

template <typename Iterator, typename BlockStruct, typename AllBlockRules, std::size_t... kIs>
auto block_or_rule_impl(const AllBlockRules& aAllBlockRules,
                        const detail::NameRule<Iterator>& aNameRule,
                        std::integer_sequence<std::size_t, kIs...> /*aSequence*/)
    -> bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>>
{
    namespace bp = boost::phoenix;
    static const auto tTerminalRule = bsq::rule<Iterator, void(), SkipperType<Iterator>>{bsq::eps(false)};
    if constexpr (kIsNamedBlock<BlockStruct>)
    {
        return bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>>{
            aNameRule[bp::at_c<0>(bsq::_val) = bsq::_1] >
            *((std::get<kIs + 1>(aAllBlockRules)[bp::at_c<kIs + 1>(bsq::_val) = bsq::_1] | ... | tTerminalRule))};
    }
    else
    {
        return bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>>{
            *((std::get<kIs>(aAllBlockRules)[bp::at_c<kIs>(bsq::_val) = bsq::_1] | ... | tTerminalRule))};
    }
}

}  // namespace detail

template <typename Iterator, typename BlockStruct, typename AllBlockRules>
auto block_or_rule(const AllBlockRules& aAllBlockRules, const detail::NameRule<Iterator>& aNameRule)
    -> bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>>
{
    constexpr auto tNumRules =
        kIsNamedBlock<BlockStruct> ? std::tuple_size_v<AllBlockRules> - 1 : std::tuple_size_v<AllBlockRules>;
    return detail::block_or_rule_impl<Iterator, BlockStruct>(aAllBlockRules, aNameRule,
                                                             std::make_index_sequence<tNumRules>{});
}

/// Generates key-value-pair rules for fields in @a BlockStruct
/// @a BlockStruct must be a struct with public members defined using a boost fusion macro.
template <typename Iterator, typename BlockStruct>
struct BlockStructRule
{
    static constexpr bool mIsNamedBlockStructRule = kIsNamedBlock<BlockStruct>;

    std::string mBlockType = InputTypeName<BlockStruct>::mName;

    detail::NameRule<Iterator> mNameRule = detail::name_identifier_rule<Iterator>();
    bsq::rule<Iterator, void(), SkipperType<Iterator>> mPreambleRule =
        bsq::lit("begin") >> bsq::lexeme[bsq::lit(mBlockType) >> !bsq::graph];
    bsq::rule<Iterator, void(), SkipperType<Iterator>> mPostambleRule = bsq::lit("end");
    BlockRuleTuple<Iterator, BlockStruct> mAllBlockRules = rule_tuple<Iterator, BlockStruct>();
    bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>> mBlockOrRule =
        block_or_rule<Iterator, BlockStruct>(mAllBlockRules, mNameRule);

    bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>> mBlockRule =
        mPreambleRule > mBlockOrRule[bsq::_val = bsq::_1] > mPostambleRule;  // NOLINT(bugprone-chained-comparison)
};

/// @brief Specifies whether a component has a name that should be parsed.
///
/// This indicates whether or not the component has a generic name and thus needs a user-given name to disambiguate.
template <components::ComponentType kComponentType>
[[maybe_unused]] constexpr auto kIsNamedComponent =
    kComponentType == components::ComponentType::kConstraint || kComponentType == components::ComponentType::kObjective;

/// @brief Specifies whether a component is unique in the input deck.
///
/// This indicates whether the component must only appear once in an input deck.
template <components::ComponentType kComponentType>
[[maybe_unused]] constexpr auto kIsUniqueComponent =
    kComponentType == components::ComponentType::kFilter || kComponentType == components::ComponentType::kGeometry;

/// @brief A parser for a struct as key-value pairs.
///
/// This is meant to be instantiated by components for their specific input types.
template <typename Iterator, typename BlockStruct, components::ComponentType kComponentType>
struct ComponentBlockRule
{
    using BlockDataStruct = BlockStruct;

    constexpr static bool mIsNamedBlockStructRule = kIsNamedComponent<kComponentType>;
    constexpr static components::ComponentType mComponentType = kComponentType;

    std::string mBlockType = InputTypeName<BlockStruct>::mName;

    detail::NameRule<Iterator> mNameRule = detail::name_identifier_rule<Iterator>();
    BlockRuleTuple<Iterator, BlockStruct> mAllBlockRules = rule_tuple<Iterator, BlockStruct>();
    bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>> mBlockOrRule =
        block_or_rule<Iterator, BlockStruct>(mAllBlockRules, mNameRule);

    bsq::rule<Iterator, BlockStruct(), SkipperType<Iterator>> mBlockRule = mBlockOrRule[bsq::_val = bsq::_1];
};

}  // namespace plato::input_parser

#endif
